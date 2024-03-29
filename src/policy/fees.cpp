// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2018-2019 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <amount.h>
#include <feerate.h>
#include <policy/fees.h>

FeeFilterRounder::FeeFilterRounder(const CFeeRate &minIncrementalFee) {
    Amount minFeeLimit = std::max(FIXOSHI, minIncrementalFee.GetFeePerK() / 2);
    feeset.insert(Amount::zero());
    for (double bucketBoundary = minFeeLimit / FIXOSHI;
         bucketBoundary <= double(MAX_FEERATE / FIXOSHI);
         bucketBoundary *= FEE_SPACING) {
        feeset.insert(int64_t(bucketBoundary) * FIXOSHI);
    }
}

Amount FeeFilterRounder::round(const Amount currentMinFee) {
    auto it = feeset.lower_bound(currentMinFee);
    if ((it != feeset.begin() && insecure_rand.rand32() % 3 != 0) ||
        it == feeset.end()) {
        it--;
    }

    return *it;
}
