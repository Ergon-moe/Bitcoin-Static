// Copyright (c) 2011-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/bitcoinunits.h>

#include <primitives/transaction.h>

#include <QStringList>

BitcoinUnits::BitcoinUnits(QObject *parent)
    : QAbstractListModel(parent), unitlist(availableUnits()) {}

QList<BitcoinUnits::Unit> BitcoinUnits::availableUnits() {
    QList<BitcoinUnits::Unit> unitlist;
    unitlist.append(XRG);
    unitlist.append(mXRG);
    unitlist.append(uXRG);
    unitlist.append(FIX);
    return unitlist;
}

bool BitcoinUnits::valid(int unit) {
    switch (unit) {
        case XRG:
        case mXRG:
        case uXRG:
        case FIX:
            return true;
        default:
            return false;
    }
}

QString BitcoinUnits::longName(int unit) {
    switch (unit) {
        case XRG:
            return QString("XRG");
        case mXRG:
            return QString("mXRG");
        case uXRG:
            return QString::fromUtf8("μXRG");
        case FIX:
            return QString("fix");
        default:
            return QString("???");
    }
}

QString BitcoinUnits::shortName(int unit) {
    switch (unit) {
        case FIX:
            return QString("fix");
        default:
            return longName(unit);
    }
}

QString BitcoinUnits::description(int unit) {
    switch (unit) {
        case XRG:
            return QString("Bitcoins");
        case mXRG:
            return QString("Milli-Bitcoins (1 / 1" THIN_SP_UTF8 "000)");
        case uXRG:
            return QString("Micro-Bitcoins (1 / 1" THIN_SP_UTF8
                           "000" THIN_SP_UTF8 "000)");
        case FIX:
            return QString("fix (1 / 100" THIN_SP_UTF8
                           "000" THIN_SP_UTF8 "000)");
        default:
            return QString("???");
    }
}

qint64 BitcoinUnits::factor(int unit) {
    switch (unit) {
        case XRG:
            return 100000000;
        case mXRG:
            return 100000;
        case uXRG:
            return 100;
        case FIX:
            return 1;
        default:
            return 100000000;
    }
}

int BitcoinUnits::decimals(int unit) {
    switch (unit) {
        case XRG:
            return 8;
        case mXRG:
            return 5;
        case uXRG:
            return 2;
        case FIX:
            return 0;
        default:
            return 0;
    }
}

QString BitcoinUnits::format(int unit, const Amount nIn, bool fPlus,
                             SeparatorStyle separators) {
    // Note: not using straight sprintf here because we do NOT want
    // localized number formatting.
    if (!valid(unit)) {
        // Refuse to format invalid unit
        return QString();
    }
    qint64 n = qint64(nIn / FIXOSHI);
    qint64 coin = factor(unit);
    int num_decimals = decimals(unit);
    qint64 n_abs = (n > 0 ? n : -n);
    qint64 quotient = n_abs / coin;
    QString quotient_str = QString::number(quotient);

    // Use SI-style thin space separators as these are locale independent and
    // can't be confused with the decimal marker.
    QChar thin_sp(THIN_SP_CP);
    int q_size = quotient_str.size();
    if (separators == separatorAlways ||
        (separators == separatorStandard && q_size > 4)) {
        for (int i = 3; i < q_size; i += 3) {
            quotient_str.insert(q_size - i, thin_sp);
        }
    }

    if (n < 0) {
        quotient_str.insert(0, '-');
    } else if (fPlus && n > 0) {
        quotient_str.insert(0, '+');
    }

    if (num_decimals > 0) {
        qint64 remainder = n_abs % coin;
        QString remainder_str =
            QString::number(remainder).rightJustified(num_decimals, '0');
        return quotient_str + QString(".") + remainder_str;
    } else {
        return quotient_str;
    }
}

// NOTE: Using formatWithUnit in an HTML context risks wrapping
// quantities at the thousands separator. More subtly, it also results
// in a standard space rather than a thin space, due to a bug in Qt's
// XML whitespace canonicalisation
//
// Please take care to use formatHtmlWithUnit instead, when
// appropriate.

QString BitcoinUnits::formatWithUnit(int unit, const Amount amount,
                                     bool plussign, SeparatorStyle separators) {
    return format(unit, amount, plussign, separators) + QString(" ") +
           shortName(unit);
}

QString BitcoinUnits::formatHtmlWithUnit(int unit, const Amount amount,
                                         bool plussign,
                                         SeparatorStyle separators) {
    QString str(formatWithUnit(unit, amount, plussign, separators));
    str.replace(QChar(THIN_SP_CP), QString(THIN_SP_HTML));
    return QString("<span style='white-space: nowrap;'>%1</span>").arg(str);
}

bool BitcoinUnits::parse(int unit, const QString &value, Amount *val_out) {
    if (!valid(unit) || value.isEmpty()) {
        // Refuse to parse invalid unit or empty string
        return false;
    }
    int num_decimals = decimals(unit);

    // Ignore spaces and thin spaces when parsing
    QStringList parts = removeSpaces(value).split(".");

    if (parts.size() > 2) {
        // More than one dot
        return false;
    }
    QString whole = parts[0];
    QString decimals;

    if (parts.size() > 1) {
        decimals = parts[1];
    }
    if (decimals.size() > num_decimals) {
        // Exceeds max precision
        return false;
    }
    bool ok = false;
    QString str = whole + decimals.leftJustified(num_decimals, '0');

    if (str.size() > 18) {
        // Longer numbers will exceed 63 bits
        return false;
    }
    Amount retvalue(int64_t(str.toLongLong(&ok)) * FIXOSHI);
    if (val_out) {
        *val_out = retvalue;
    }
    return ok;
}

QString BitcoinUnits::getAmountColumnTitle(int unit) {
    QString amountTitle = QObject::tr("Amount");
    if (BitcoinUnits::valid(unit)) {
        amountTitle += " (" + BitcoinUnits::shortName(unit) + ")";
    }
    return amountTitle;
}

int BitcoinUnits::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return unitlist.size();
}

QVariant BitcoinUnits::data(const QModelIndex &index, int role) const {
    int row = index.row();
    if (row >= 0 && row < unitlist.size()) {
        Unit unit = unitlist.at(row);
        switch (role) {
            case Qt::EditRole:
            case Qt::DisplayRole:
                return QVariant(longName(unit));
            case Qt::ToolTipRole:
                return QVariant(description(unit));
            case UnitRole:
                return QVariant(static_cast<int>(unit));
        }
    }
    return QVariant();
}

Amount BitcoinUnits::maxMoney() {
    return MAX_MONEY;
}
