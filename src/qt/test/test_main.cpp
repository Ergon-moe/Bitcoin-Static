// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include <config/bitcoin-config.h>
#endif

#include <chainparams.h>
#include <compat/setenv.h>
#include <interfaces/node.h>
#include <key.h>
#include <util/system.h>

#include <qt/bitcoin.h>
#include <qt/test/apptests.h>
#include <qt/test/bitcoinaddressvalidatortests.h>
#include <qt/test/compattests.h>
#include <qt/test/guiutiltests.h>
#include <qt/test/rpcnestedtests.h>
#include <qt/test/uritests.h>
#ifdef ENABLE_WALLET
#include <qt/test/addressbooktests.h>
#ifdef ENABLE_BIP70
#include <qt/test/paymentservertests.h>
#endif // ENABLE_BIP70
#include <qt/test/wallettests.h>
#endif // ENABLE_WALLET

#include <QApplication>
#include <QObject>
#include <QTest>

#ifdef ENABLE_BIP70
#include <openssl/ssl.h>
#endif

#if defined(QT_STATICPLUGIN)
#include <QtPlugin>
#if defined(QT_QPA_PLATFORM_MINIMAL)
Q_IMPORT_PLUGIN(QMinimalIntegrationPlugin);
#endif
#if defined(QT_QPA_PLATFORM_XCB)
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin);
#elif defined(QT_QPA_PLATFORM_WINDOWS)
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#elif defined(QT_QPA_PLATFORM_COCOA)
Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin);
#endif
#endif

extern void noui_connect();

// This is all you need to run all the tests
int main(int argc, char *argv[]) {
    SetupEnvironment();
    SetupNetworking();
    SelectParams(CBaseChainParams::REGTEST);
    noui_connect();
    ClearDatadirCache();
    fs::path pathTemp =
        fs::temp_directory_path() / strprintf("test_bitcoin-qt_%lu_%i",
                                              (unsigned long)GetTime(),
                                              (int)GetRand(100000));
    fs::create_directories(pathTemp);
    gArgs.ForceSetArg("-datadir", pathTemp.string());
    auto node = interfaces::MakeNode();

    bool fInvalid = false;

    // Prefer the "minimal" platform for the test instead of the normal default
    // platform ("xcb", "windows", or "cocoa") so tests can't unintentionally
    // interfere with any background GUIs and don't require extra resources.
    setenv("QT_QPA_PLATFORM", "minimal", 0);

    // Don't remove this, it's needed to access
    // QApplication:: and QCoreApplication:: in the tests
    BitcoinApplication app(*node, argc, argv);
    app.setApplicationName("BitcoinStatic-Qt-test");

#ifdef ENABLE_BIP70
    // This is necessary to initialize openssl on the test framework
    // (at least on Darwin).
    SSL_library_init();
#endif

    AppTests app_tests(app);
    if (QTest::qExec(&app_tests) != 0) {
        fInvalid = true;
    }
    URITests test1;
    if (QTest::qExec(&test1) != 0) {
        fInvalid = true;
    }
#if defined(ENABLE_WALLET) && defined(ENABLE_BIP70)
    PaymentServerTests test2;
    if (QTest::qExec(&test2) != 0) {
        fInvalid = true;
    }
#endif
    RPCNestedTests test3;
    if (QTest::qExec(&test3) != 0) {
        fInvalid = true;
    }
    CompatTests test4;
    if (QTest::qExec(&test4) != 0) {
        fInvalid = true;
    }
    GUIUtilTests test5;
    if (QTest::qExec(&test5) != 0) {
        fInvalid = true;
    }
    BitcoinAddressValidatorTests test6;
    if (QTest::qExec(&test6) != 0) {
        fInvalid = true;
    }
#ifdef ENABLE_WALLET
    WalletTests test7;
    if (QTest::qExec(&test7) != 0) {
        fInvalid = true;
    }
    AddressBookTests test8;
    if (QTest::qExec(&test8) != 0) {
        fInvalid = true;
    }
#endif

    fs::remove_all(pathTemp);

    return fInvalid;
}
