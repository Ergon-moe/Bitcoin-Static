// Copyright (c) 2019 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chain.h>
#include <chainparams.h>
#include <consensus/activation.h>
#include <util/system.h>

#include <test/setup_common.h>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(activation_tests, BasicTestingSetup)

static void SetMTP(std::array<CBlockIndex, 12> &blocks, int64_t mtp) {
    size_t len = blocks.size();

    for (size_t i = 0; i < len; ++i) {
        blocks[i].nTime = mtp + (i - (len / 2));
    }

    BOOST_CHECK_EQUAL(blocks.back().GetMedianTimePast(), mtp);
}

BOOST_AUTO_TEST_CASE(isgravitonenabled) {
    const auto params = CreateChainParams(CBaseChainParams::MAIN);
    const auto &consensus = params->GetConsensus();

    BOOST_CHECK(!IsGravitonEnabled(consensus, nullptr));

    std::array<CBlockIndex, 4> blocks;
    blocks[0].nHeight = consensus.gravitonHeight - 2;
    for (size_t i = 1; i < blocks.size(); ++i) {
        blocks[i].pprev = &blocks[i - 1];
        blocks[i].nHeight = blocks[i - 1].nHeight + 1;
    }
    BOOST_CHECK(!IsGravitonEnabled(consensus, &blocks[0]));
    BOOST_CHECK(!IsGravitonEnabled(consensus, &blocks[1]));
    BOOST_CHECK(IsGravitonEnabled(consensus, &blocks[2]));
    BOOST_CHECK(IsGravitonEnabled(consensus, &blocks[3]));
}

BOOST_AUTO_TEST_CASE(isphononenabled) {
    const Consensus::Params &consensus = Params().GetConsensus();
    BOOST_CHECK(!IsPhononEnabled(consensus, nullptr));

    std::array<CBlockIndex, 4> blocks;
    blocks[0].nHeight = consensus.phononHeight - 2;

    for (size_t i = 1; i < blocks.size(); ++i) {
        blocks[i].pprev = &blocks[i - 1];
        blocks[i].nHeight = blocks[i - 1].nHeight + 1;
    }
    BOOST_CHECK(!IsPhononEnabled(consensus, &blocks[0]));
    BOOST_CHECK(!IsPhononEnabled(consensus, &blocks[1]));
    BOOST_CHECK(IsPhononEnabled(consensus, &blocks[2]));
    BOOST_CHECK(IsPhononEnabled(consensus, &blocks[3]));
}

BOOST_AUTO_TEST_CASE(isaxionenabled) {
    // first, test chains with no hard-coded activation height (activation based on MTP)
    {
        const auto pparams = CreateChainParams(CBaseChainParams::SCALENET);
        const Consensus::Params &params = pparams->GetConsensus();
        const auto activation =
            gArgs.GetArg("-axionactivationtime", params.axionActivationTime);
        SetMockTime(activation - 1000000);

        BOOST_CHECK(!IsAxionEnabled(params, nullptr));

        std::array<CBlockIndex, 12> blocks;
        for (size_t i = 1; i < blocks.size(); ++i) {
            blocks[i].pprev = &blocks[i - 1];
        }
        BOOST_CHECK(!IsAxionEnabled(params, &blocks.back()));

        SetMTP(blocks, activation - 1);
        BOOST_CHECK(!IsAxionEnabled(params, &blocks.back()));

        SetMTP(blocks, activation);
        BOOST_CHECK(IsAxionEnabled(params, &blocks.back()));

        SetMTP(blocks, activation + 1);
        BOOST_CHECK(IsAxionEnabled(params, &blocks.back()));
    }

    // next, test chains with height-based activation
    {
        const auto pparams = CreateChainParams(CBaseChainParams::MAIN);
        const auto &params = pparams->GetConsensus();
        const auto axionHeight = params.asertAnchorParams->nHeight;

        std::array<CBlockIndex, 4> blocks;
        blocks[0].nHeight = axionHeight - 2;
        for (size_t i = 1; i < blocks.size(); ++i) {
            blocks[i].pprev = &blocks[i - 1];
            blocks[i].nHeight = blocks[i - 1].nHeight + 1;
        }
        BOOST_CHECK(!IsAxionEnabled(params, &blocks[0]));
        BOOST_CHECK(!IsAxionEnabled(params, &blocks[1]));
        BOOST_CHECK(IsAxionEnabled(params, &blocks[2]));
        BOOST_CHECK(IsAxionEnabled(params, &blocks[3]));
    }
}

BOOST_AUTO_TEST_CASE(istachyonenabled) {
    CBlockIndex prev;

    const Consensus::Params &params = Params().GetConsensus();
    const auto activation =
        gArgs.GetArg("-tachyonctivationtime", params.tachyonActivationTime);
    SetMockTime(activation - 1000000);

    BOOST_CHECK(!IsTachyonEnabled(params, nullptr));

    std::array<CBlockIndex, 12> blocks;
    for (size_t i = 1; i < blocks.size(); ++i) {
        blocks[i].pprev = &blocks[i - 1];
    }
    BOOST_CHECK(!IsTachyonEnabled(params, &blocks.back()));

    SetMTP(blocks, activation - 1);
    BOOST_CHECK(!IsTachyonEnabled(params, &blocks.back()));

    SetMTP(blocks, activation);
    BOOST_CHECK(IsTachyonEnabled(params, &blocks.back()));

    SetMTP(blocks, activation + 1);
    BOOST_CHECK(IsTachyonEnabled(params, &blocks.back()));
}

BOOST_AUTO_TEST_CASE(isemaenabled) {
    CBlockIndex prev;

    const Consensus::Params &params = Params().GetConsensus();
    const auto activation =
        gArgs.GetArg("-emaationtime", params.tachyonActivationTime);
    SetMockTime(activation - 1000000);

    BOOST_CHECK(!IsEMAEnabled(params, nullptr));

    std::array<CBlockIndex, 12> blocks;
    for (size_t i = 1; i < blocks.size(); ++i) {
        blocks[i].pprev = &blocks[i - 1];
    }
    BOOST_CHECK(!IsEMAEnabled(params, &blocks.back()));

    SetMTP(blocks, activation - 1);
    BOOST_CHECK(!IsEMAEnabled(params, &blocks.back()));

    SetMTP(blocks, activation);
    BOOST_CHECK(IsEMAEnabled(params, &blocks.back()));

    SetMTP(blocks, activation + 1);
    BOOST_CHECK(IsEMAEnabled(params, &blocks.back()));
}

BOOST_AUTO_TEST_SUITE_END()
