Bitcoin Static Release Process
=================================


## Before Release

1. Check configuration
    - Check features planned for the release are implemented and documented
      (or more informally, that the Release Manager agrees it is feature complete)
    - Check that finished tasks / tickets are marked as resolved

2. Verify tests passed
    - Any known issues or limitations should be documented in release notes
    - Known bugs should have tickets
    - Run `arc lint --everything` and check there is no linter error
    - Ensure that bitcoind and bitcoin-qt run with no issue on all supported platforms.
      Manually test bitcoin-qt by sending some transactions and navigating through the menus.

3. Update the documents / code which needs to be updated every release
    - Check that [release-notes.md](release-notes.md) is complete, and fill in any missing items.
    - Update [bips.md](bips.md) to account for changes since the last release.
    - (major releases) Update [`BLOCK_CHAIN_SIZE`](../src/qt/intro.cpp) to the current size plus
      some overhead.
    - Regenerate manpages (run `contrib/devtools/gen-manpages.sh`, or for out-of-tree builds run
      `BUILDDIR=$PWD/build contrib/devtools/gen-manpages.sh`).
    - Update seeds as per [contrib/seeds/README.md](../contrib/seeds/README.md).
    - Update [`src/chainparams.cpp`](../src/chainparams.cpp) m_assumed_blockchain_size and m_assumed_chain_state_size with the current size plus some overhead.(see [this](#how-to-calculate-assumed-blockchain-and-chain-state-size) for information on how to calculate them).


4. Increment version number for the next release in:
    - `doc/release-notes.md` (and copy existing one to versioned `doc/release-notes/*.md`)
    - `configure.ac`
    - `CMakeLists.txt`
    - `contrib/seeds/makeseeds.py` (only after a new major release)

5. Add git tag for release
    a. Create the tag: `git tag -s vM.m.r` (M = major version, m = minor version, r = revision)
    b. Push the tag to GitLab:

```
git push <github remote> master
git push <github remote> --tags -f
```

## Release

6. Create Gitian Builds (see [gitian-building.md](gitian-building.md))

7. Verify matching Gitian Builds, gather signatures

8. Verify IBD bith with and without `-checkpoints=0 -assumevalid=0`

9. Upload Gitian Builds to [ergon.moe](https://ergon.moe/)

10. Create a [release](https://github.com/Ergon-Moe/Bitcoin-Static) on our GitHub mirror:
    `contrib/release/github-release.sh -a <path to release binaries> -t <release tag> -o <file containing your Github OAuth token>`

11. Create [Ubuntu PPA packages](https://launchpad.net/~bitcoin-cash-node/+archive/ubuntu/ppa):
    Maintainers need to clone [packaging](https://gitlab.com/bitcoin-cash-node/bchn-sw/packaging)
    and follow instructions to run `debian-packaging.sh` in that repository.

12. Notify maintainers of AUR and Docker images to build their packages.
    They should be given 1-day advance notice if possible.

## After Release

12. Update version number on www.ergon.moe

13. Publish signed checksums (various places, e.g. blog, reddit, etc. etc.)

14. Announce Release:
    - [Reddit](https://www.reddit.com/r/ergon/)
    - [Telegram](https://t.me/ErgonCommunity)
    - Discord
### Additional information

#### <a name="how-to-calculate-assumed-blockchain-and-chain-state-size"></a>How to calculate `m_assumed_blockchain_size` and `m_assumed_chain_state_size`

Both variables are used as a guideline for how much space the user needs on their drive in total, not just strictly for the blockchain.
Note that all values should be taken from a **fully synced** node and have an overhead of 5-10% added on top of its base value.

To calculate `m_assumed_blockchain_size`, take the size in GiB of these directories:
- For `mainnet` -> the data directory, excluding the `/testnet3`, `/signet`, and `/regtest` directories and any overly large files, e.g. a huge `debug.log`
- For `testnet` -> `/testnet3`
- For `signet` -> `/signet`

To calculate `m_assumed_chain_state_size`, take the size in GiB of these directories:
- For `mainnet` -> `/chainstate`
- For `testnet` -> `/testnet3/chainstate`
- For `signet` -> `/signet/chainstate`

Notes:
- When taking the size for `m_assumed_blockchain_size`, there's no need to exclude the `/chainstate` directory since it's a guideline value and an overhead will be added anyway.
- The expected overhead for growth may change over time. Consider whether the percentage needs to be changed in response; if so, update it here in this section.
