// Copyright (c) 2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <amount.h>

#include <test/setup_common.h>

#include <boost/test/unit_test.hpp>

#include <array>

BOOST_FIXTURE_TEST_SUITE(feerate_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(GetFeeTest) {
    CFeeRate feeRate, altFeeRate;

    feeRate = CFeeRate(Amount::zero());
    // Must always return 0
    BOOST_CHECK_EQUAL(feeRate.GetFee(0), Amount::zero());
    BOOST_CHECK_EQUAL(feeRate.GetFee(1e5), Amount::zero());

    feeRate = CFeeRate(1000 * FIXOSHI);
    // Must always just return the arg
    BOOST_CHECK_EQUAL(feeRate.GetFee(0), Amount::zero());
    BOOST_CHECK_EQUAL(feeRate.GetFee(1), FIXOSHI);
    BOOST_CHECK_EQUAL(feeRate.GetFee(121), 121 * FIXOSHI);
    BOOST_CHECK_EQUAL(feeRate.GetFee(999), 999 * FIXOSHI);
    BOOST_CHECK_EQUAL(feeRate.GetFee(1000), 1000 * FIXOSHI);
    BOOST_CHECK_EQUAL(feeRate.GetFee(9000), 9000 * FIXOSHI);

    feeRate = CFeeRate(-1000 * FIXOSHI);
    // Must always just return -1 * arg
    BOOST_CHECK_EQUAL(feeRate.GetFee(0), Amount::zero());
    BOOST_CHECK_EQUAL(feeRate.GetFee(1), -FIXOSHI);
    BOOST_CHECK_EQUAL(feeRate.GetFee(121), -121 * FIXOSHI);
    BOOST_CHECK_EQUAL(feeRate.GetFee(999), -999 * FIXOSHI);
    BOOST_CHECK_EQUAL(feeRate.GetFee(1000), -1000 * FIXOSHI);
    BOOST_CHECK_EQUAL(feeRate.GetFee(9000), -9000 * FIXOSHI);

    feeRate = CFeeRate(123 * FIXOSHI);
    // Truncates the result, if not integer
    BOOST_CHECK_EQUAL(feeRate.GetFee(0), Amount::zero());
    // Special case: returns 1 instead of 0
    BOOST_CHECK_EQUAL(feeRate.GetFee(8), FIXOSHI);
    BOOST_CHECK_EQUAL(feeRate.GetFee(9), FIXOSHI);
    BOOST_CHECK_EQUAL(feeRate.GetFee(121), 14 * FIXOSHI);
    BOOST_CHECK_EQUAL(feeRate.GetFee(122), 15 * FIXOSHI);
    BOOST_CHECK_EQUAL(feeRate.GetFee(999), 122 * FIXOSHI);
    BOOST_CHECK_EQUAL(feeRate.GetFee(1000), 123 * FIXOSHI);
    BOOST_CHECK_EQUAL(feeRate.GetFee(9000), 1107 * FIXOSHI);

    feeRate = CFeeRate(-123 * FIXOSHI);
    // Truncates the result, if not integer
    BOOST_CHECK_EQUAL(feeRate.GetFee(0), Amount::zero());
    // Special case: returns -1 instead of 0
    BOOST_CHECK_EQUAL(feeRate.GetFee(8), -FIXOSHI);
    BOOST_CHECK_EQUAL(feeRate.GetFee(9), -FIXOSHI);

    // Check ceiling results
    feeRate = CFeeRate(18 * FIXOSHI);
    // Truncates the result, if not integer
    BOOST_CHECK_EQUAL(feeRate.GetFeeCeiling(0), Amount::zero());
    BOOST_CHECK_EQUAL(feeRate.GetFeeCeiling(100), 2 * FIXOSHI);
    BOOST_CHECK_EQUAL(feeRate.GetFeeCeiling(200), 4 * FIXOSHI);
    BOOST_CHECK_EQUAL(feeRate.GetFeeCeiling(1000), 18 * FIXOSHI);

    // Check alternate constructor
    feeRate = CFeeRate(1000 * FIXOSHI);
    altFeeRate = CFeeRate(feeRate);
    BOOST_CHECK_EQUAL(feeRate.GetFee(100), altFeeRate.GetFee(100));

    // Check full constructor
    BOOST_CHECK(CFeeRate(-FIXOSHI, 0) == CFeeRate(Amount::zero()));
    BOOST_CHECK(CFeeRate(Amount::zero(), 0) == CFeeRate(Amount::zero()));
    BOOST_CHECK(CFeeRate(FIXOSHI, 0) == CFeeRate(Amount::zero()));
    // default value
    BOOST_CHECK(CFeeRate(-FIXOSHI, 1000) == CFeeRate(-FIXOSHI));
    BOOST_CHECK(CFeeRate(Amount::zero(), 1000) == CFeeRate(Amount::zero()));
    BOOST_CHECK(CFeeRate(FIXOSHI, 1000) == CFeeRate(FIXOSHI));
    // lost precision (can only resolve satoshis per kB)
    BOOST_CHECK(CFeeRate(FIXOSHI, 1001) == CFeeRate(Amount::zero()));
    BOOST_CHECK(CFeeRate(2 * FIXOSHI, 1001) == CFeeRate(FIXOSHI));
    // some more integer checks
    BOOST_CHECK(CFeeRate(26 * FIXOSHI, 789) == CFeeRate(32 * FIXOSHI));
    BOOST_CHECK(CFeeRate(27 * FIXOSHI, 789) == CFeeRate(34 * FIXOSHI));
    // Maximum size in bytes, should not crash
    CFeeRate(MAX_MONEY, std::numeric_limits<size_t>::max() >> 1).GetFeePerK();
}

BOOST_AUTO_TEST_SUITE_END()
