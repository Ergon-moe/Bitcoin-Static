// Copyright (c) 2015 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chain.h>
#include <chainparams.h>
#include <config.h>
#include <consensus/activation.h>
#include <pow.h>
#include <random.h>
#include <tinyformat.h>
#include <util/system.h>

#include <test/setup_common.h>

#include <boost/test/unit_test.hpp>

#include <cmath>
#include <memory>
#include <string>

BOOST_FIXTURE_TEST_SUITE(pow_tests, TestingSetup)

/* Test calculation of next difficulty target with no constraints applying */
BOOST_AUTO_TEST_CASE(get_next_work) {
    DummyConfig config(CBaseChainParams::MAIN);

    int64_t nLastRetargetTime = 1261130161; // Block #30240
    CBlockIndex pindexLast;
    pindexLast.nHeight = 32255;
    pindexLast.nTime = 1262152739; // Block #32255
    pindexLast.nBits = 0x1d00ffff;
    BOOST_CHECK_EQUAL(
        CalculateNextWorkRequired(&pindexLast, nLastRetargetTime,
                                  config.GetChainParams().GetConsensus()),
        0x1d00d86aU);
}

/* Test the constraint on the upper bound for next work */
BOOST_AUTO_TEST_CASE(get_next_work_pow_limit) {
    DummyConfig config(CBaseChainParams::MAIN);

    int64_t nLastRetargetTime = 1231006505; // Block #0
    CBlockIndex pindexLast;
    pindexLast.nHeight = 2015;
    pindexLast.nTime = 1233061996; // Block #2015
    pindexLast.nBits = 0x1d0fffff;
    BOOST_CHECK_EQUAL(
        CalculateNextWorkRequired(&pindexLast, nLastRetargetTime,
                                  config.GetChainParams().GetConsensus()),
        0x1d0fffffU);
}

/* Test the constraint on the lower bound for actual time taken */
BOOST_AUTO_TEST_CASE(get_next_work_lower_limit_actual) {
    DummyConfig config(CBaseChainParams::MAIN);

    int64_t nLastRetargetTime = 1279008237; // Block #66528
    CBlockIndex pindexLast;
    pindexLast.nHeight = 68543;
    pindexLast.nTime = 1279297671; // Block #68543
    pindexLast.nBits = 0x1c05a3f4;
    BOOST_CHECK_EQUAL(
        CalculateNextWorkRequired(&pindexLast, nLastRetargetTime,
                                  config.GetChainParams().GetConsensus()),
        0x1c0168fdU);
}

/* Test the constraint on the upper bound for actual time taken */
BOOST_AUTO_TEST_CASE(get_next_work_upper_limit_actual) {
    DummyConfig config(CBaseChainParams::MAIN);

    int64_t nLastRetargetTime = 1263163443; // NOTE: Not an actual block time
    CBlockIndex pindexLast;
    pindexLast.nHeight = 46367;
    pindexLast.nTime = 1269211443; // Block #46367
    pindexLast.nBits = 0x1c387f6f;
    BOOST_CHECK_EQUAL(
        CalculateNextWorkRequired(&pindexLast, nLastRetargetTime,
                                  config.GetChainParams().GetConsensus()),
        0x1d00e1fdU);
}

BOOST_AUTO_TEST_CASE(GetBlockProofEquivalentTime_test) {
    DummyConfig config(CBaseChainParams::MAIN);

    std::vector<CBlockIndex> blocks(10000);
    for (int i = 0; i < 10000; i++) {
        blocks[i].pprev = i ? &blocks[i - 1] : nullptr;
        blocks[i].nHeight = i;
        blocks[i].nTime =
            1269211443 +
            i * config.GetChainParams().GetConsensus().nPowTargetSpacing;
        blocks[i].nBits = 0x207fffff; /* target 0x7fffff000... */
        blocks[i].nChainWork =
            i ? blocks[i - 1].nChainWork + GetBlockProof(blocks[i])
              : arith_uint256(0);
    }

    for (int j = 0; j < 1000; j++) {
        CBlockIndex *p1 = &blocks[InsecureRandRange(10000)];
        CBlockIndex *p2 = &blocks[InsecureRandRange(10000)];
        CBlockIndex *p3 = &blocks[InsecureRandRange(10000)];

        int64_t tdiff = GetBlockProofEquivalentTime(
            *p1, *p2, *p3, config.GetChainParams().GetConsensus());
        BOOST_CHECK_EQUAL(tdiff, p1->GetBlockTime() - p2->GetBlockTime());
    }
}

using CBlockIndexPtr = std::unique_ptr<CBlockIndex>;
const auto MkCBlockIndexPtr = &std::make_unique<CBlockIndex>;

static CBlockIndexPtr GetBlockIndex(CBlockIndex *pindexPrev, int64_t nTimeInterval,
                                    uint32_t nBits) {
    CBlockIndexPtr block = MkCBlockIndexPtr();
    block->pprev = pindexPrev;
    block->nHeight = pindexPrev->nHeight + 1;
    block->nTime = pindexPrev->nTime + nTimeInterval;
    block->nBits = nBits;

    block->BuildSkip();
    block->nChainWork = pindexPrev->nChainWork + GetBlockProof(*block);
    return block;
}

BOOST_AUTO_TEST_CASE(exp_difficulty_test) {
    DummyConfig config(CBaseChainParams::MAIN);

    std::vector<CBlockIndexPtr> blocks(3000);

    const Consensus::Params &params = config.GetChainParams().GetConsensus();
    const arith_uint256 powLimit = UintToArith256(params.powLimit);
    uint32_t powLimitBits = powLimit.GetCompact();
    arith_uint256 currentPow = powLimit >> 4;
    uint32_t initialBits = currentPow.GetCompact();

    // Genesis block.
    blocks[0] = MkCBlockIndexPtr();
    blocks[0]->nHeight = 0;
    blocks[0]->nTime = 1269211443;
    blocks[0]->nBits = initialBits;

    blocks[0]->nChainWork = GetBlockProof(*blocks[0]);

    // Block counter.
    size_t i;

    // Pile up some blocks every 10 mins to establish some history.
    for (i = 1; i < 2050; i++) {
        blocks[i] = GetBlockIndex(blocks[i - 1].get(), 600, initialBits);
    }

    CBlockHeader blkHeaderDummy;
    uint32_t nBits =
        GetNextExpWorkRequired(blocks[2049].get(), &blkHeaderDummy, params);

    // Difficulty stays the same as long as we produce a block every 10 mins.
    for (size_t j = 0; j < 10; i++, j++) {
        blocks[i] = GetBlockIndex(blocks[i - 1].get(), 600, nBits);
        BOOST_CHECK_EQUAL(
            GetNextExpWorkRequired(blocks[i].get(), &blkHeaderDummy, params),
            nBits);
    }

    // Make sure we skip over blocks that are out of wack. To do so, we produce
    // a block that is far in the future, and then produce a block with the
    // expected timestamp.
    blocks[i] = GetBlockIndex(blocks[i - 1].get(), 6000, nBits);
    BOOST_CHECK_EQUAL(
        GetNextExpWorkRequired(blocks[i++].get(), &blkHeaderDummy, params), nBits);
    blocks[i] = GetBlockIndex(blocks[i - 1].get(), 2*600 - 6000, nBits);
    i++;
    blocks[i] = GetBlockIndex(blocks[i - 1].get(), 600, nBits);
    i++;
    blocks[i] = GetBlockIndex(blocks[i - 1].get(), 600, nBits);
    i++;
    blocks[i] = GetBlockIndex(blocks[i - 1].get(), 600, nBits);
    BOOST_CHECK_EQUAL(
        GetNextExpWorkRequired(blocks[i++].get(), &blkHeaderDummy, params), nBits);

    // The system should continue unaffected by the block with a bogous
    // timestamps.
    for (size_t j = 0; j < 20; i++, j++) {
        blocks[i] = GetBlockIndex(blocks[i - 1].get(), 600, nBits);
        BOOST_CHECK_EQUAL(
            GetNextExpWorkRequired(blocks[i].get(), &blkHeaderDummy, params),
            nBits);
    }

    // We start emitting blocks slightly faster. The first block has no impact.
    blocks[i] = GetBlockIndex(blocks[i - 1].get(), 550, nBits);
    BOOST_CHECK_EQUAL(
        GetNextExpWorkRequired(blocks[i++].get(), &blkHeaderDummy, params), nBits);

    // Now we should see difficulty increase slowly.
    for (size_t j = 0; j < 10; i++, j++) {
        blocks[i] = GetBlockIndex(blocks[i - 1].get(), 550, nBits);
        const uint32_t nextBits =
            GetNextExpWorkRequired(blocks[i].get(), &blkHeaderDummy, params);

        arith_uint256 currentTarget;
        currentTarget.SetCompact(nBits);
        arith_uint256 nextTarget;
        nextTarget.SetCompact(nextBits);

        // Make sure that difficulty increases very slowly.
        BOOST_CHECK(nextTarget < currentTarget);
        BOOST_CHECK((currentTarget - nextTarget) < (currentTarget >> 10));

        nBits = nextBits;
    }

    // If we dramatically shorten block production, difficulty increases faster.
    for (size_t j = 0; j < 20; i++, j++) {
        blocks[i] = GetBlockIndex(blocks[i - 1].get(), 10, nBits);
        const uint32_t nextBits =
            GetNextExpWorkRequired(blocks[i].get(), &blkHeaderDummy, params);

        arith_uint256 currentTarget;
        currentTarget.SetCompact(nBits);
        arith_uint256 nextTarget;
        nextTarget.SetCompact(nextBits);

        // Make sure that difficulty increases faster.
        BOOST_CHECK(nextTarget < currentTarget);
        BOOST_CHECK((currentTarget - nextTarget) < (currentTarget >> 4));

        nBits = nextBits;
    }

    // We start to emit blocks significantly slower. The first block has no
    // impact.
    blocks[i] = GetBlockIndex(blocks[i - 1].get(), 6000, nBits);
    nBits = GetNextExpWorkRequired(blocks[i++].get(), &blkHeaderDummy, params);

    // If we dramatically slow down block production, difficulty decreases.
    for (size_t j = 0; j < 93; i++, j++) {
        blocks[i] = GetBlockIndex(blocks[i - 1].get(), 6000, nBits);
        const uint32_t nextBits =
            GetNextExpWorkRequired(blocks[i].get(), &blkHeaderDummy, params);

        arith_uint256 currentTarget;
        currentTarget.SetCompact(nBits);
        arith_uint256 nextTarget;
        nextTarget.SetCompact(nextBits);

        // Check the difficulty decreases.
        BOOST_CHECK(nextTarget <= powLimit);
        BOOST_CHECK(nextTarget > currentTarget);
        BOOST_CHECK((nextTarget - currentTarget) < (currentTarget >> 3));

        nBits = nextBits;
    }

    // Due to the window of time being bounded, next block's difficulty actually
    // gets harder.
    blocks[i] = GetBlockIndex(blocks[i - 1].get(), 6000, nBits);
    nBits = GetNextExpWorkRequired(blocks[i++].get(), &blkHeaderDummy, params);

    // And goes down again. It takes a while due to the window being bounded and
    // the skewed block causes 2 blocks to get out of the window.
    for (size_t j = 0; j < 218; i++, j++) {
        blocks[i] = GetBlockIndex(blocks[i - 1].get(), 6000, nBits);
        const uint32_t nextBits =
            GetNextExpWorkRequired(blocks[i].get(), &blkHeaderDummy, params);

        arith_uint256 currentTarget;
        currentTarget.SetCompact(nBits);
        arith_uint256 nextTarget;
        nextTarget.SetCompact(nextBits);

        // Check the difficulty decreases.
        BOOST_CHECK(nextTarget <= powLimit);
        BOOST_CHECK(nextTarget > currentTarget);
        BOOST_CHECK((nextTarget - currentTarget) < (currentTarget >> 3));

        nBits = nextBits;
    }

    // Once the difficulty reached the minimum allowed level, it doesn't get any
    // easier.
    for (size_t j = 0; j < 5; i++, j++) {
        blocks[i] = GetBlockIndex(blocks[i - 1].get(), 6000, nBits);
        const uint32_t nextBits =
            GetNextExpWorkRequired(blocks[i].get(), &blkHeaderDummy, params);

        // Check the difficulty stays constant.
        BOOST_CHECK_EQUAL(nextBits, powLimitBits);
        nBits = nextBits;
    }
}

double TargetFromBits(const uint32_t nBits) {
    return (nBits & 0xff'ff'ff) * pow(256, (nBits >> 24)-3);
}


BOOST_AUTO_TEST_SUITE_END()
