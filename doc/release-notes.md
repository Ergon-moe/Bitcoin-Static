# Release Notes for Bitcoin Static version 23.1.1

Bitcoin Static version 23.1.1 is now available from:

  <https://github.com/Ergon-moe/Bitcoin-Static/releases/>

## Overview

Default node policy reduced to 0.01 fix/b. Default data falder changed to bitcoin-static, **see usage recommendations for the migration guide**. Some more rebranding fixes and full C++17 migration.



## Usage recommendations

igration guide from 23.0.0 to 23.1.1

The default data folder has been changed from Bitcoin to Bitcoin Static. It means that the version 23.1.0 won't be able to see your coins unless you. The recommended way to migrate between those version:

(optional)
0. The new version implements a reduced transaction fee. If you have many utxos that might need combining and you want to do it on the older node version, add the following options to your bitcoin.conf file.
dustrelayfee=0.0000001
minrelaytxfee=0.0000001
blockmintxfee=0.0000001
mintxfee=0.0000001

1. Download 23.1.0 but keep the 23.0.0 binary file for your architecture,
2. Turn on the Bitcoin Static qt wallet and sellect the default directory,
3. Sellect the receive tab, click request payment and copy the address and paste it to a text file,
4. Close the Bitcoin Static window and turn on Bitcoin Static 23.0.0,
3. Go to Send tab and send all the coins to the address.

If you prefer to keep the addresses you're using, you need to export private keys corresponding to the addresses and import them to the new version.

## Network changes

...


## Added functionality

...


## Deprecated functionality

...


## Removed functionality

...


## New RPC methods

...


## Low-level RPC changes

...


## Regressions

...


## Known Issues

Inherited from Bitcoin Cash Node:

- MacOS versions earlier than 10.12 are no longer supported. Additionally,
  Bitcoin Static does not yet change appearance when macOS "dark mode"
  is activated.

- Windows users are recommended not to run multiple instances of bitcoin-qt
  or bitcoind on the same machine if the wallet feature is enabled.
  There is risk of data corruption if instances are configured to use the same
  wallet folder.

- Some users have encountered unit tests failures when running in WSL
  environments (e.g. WSL/Ubuntu).  At this time, WSL is not considered a
  supported environment for the software. This may change in future.

  The functional failure on WSL is tracked in Issue #33.
  It arises when competing node program instances are not prevented from
  opening the same wallet folder. Running multiple program instances with
  the same configured walletdir could potentially lead to data corruption.
  The failure has not been observed on other operating systems so far.

- `doc/dependencies.md` needs revision (Issue #65).

- `test_bitcoin` can collide with temporary files if used by more than
  one user on the same system simultaneously. (Issue #43)

- For users running from sources built with BerkeleyDB releases newer than
  the 5.3 which is used in this release, please take into consideration
  the database format compatibility issues described in Issue #34.
  When building from source it is recommended to use BerkeleyDB 5.3 as this
  avoids wallet database incompatibility issues with the official release.

- There is a documentation build bug that causes some ordered lists on
  docs.bitcoincashnode.org to be rendered incorrectly (Issue #141).

- The `test_bitcoin-qt` test executable fails on Linux Mint 20
  (see Issue #144). This does not otherwise appear to impact the functioning
  of the ERGN software on that platform.

- An 'autotools' build (the old build method) fails on OSX when using Clang.
  (Issue #129)

- With a certain combination of build flags that included disabling
  the QR code library, a build failure was observed where an erroneous
  linking against the QR code library (not present) was attempted (Issue #138).

- The 'autotools' build is known to require some workarounds in order to
  use the 'fuzzing' build feature (Issue #127).

- Some functional tests are known to fail spuriously with varying probability.
  (see e.g. issue #148, and a fuller listing in #162).

- Possible out-of-memory error when starting bitcoind with high excessiveblocksize
  value (Issue #156)

- There are obstacles building the current version on Ubuntu 16.04 (see #187).
  These are worked around by our packaging system, but users trying to build
  the software from scratch are advised to either upgrade to more recent Ubuntu,
  or retrofit the necessary toolchains and perform the same build steps for
  Xenial as registered in our packaging repository, or build in a VM using
  the gitian build instructions, or run our reproducible binary release builds.

- The `getnetworkhashps` RPC call is not adapted to per-block DAA (see #193).
  It has an option to calculate average hashrate "since last difficulty change"
  and wrongly assumes difficulty changes every 2016 blocks. This irrelevant
  option will likely be removed in the next release.

- A problem was observed on scalenet where nodes would sometimes hang for
  around 10 minutes, accepting RPC connections but not responding to them
  (see #210).

- `arc lint` will advise that some `src/` files are in need of reformatting
  or contain errors. - this is because code style checking is currently a work in
  progress while we adjust it to our own project requirements (see Issue #75).
  One file in `doc` also violates the lint tool (Issue #153), and a new
  script in `test/benchmark/` likewise contains code that is flagged by
  the current linting configuration. There are also RPC parameter type
  inconsistencies that flag linting checks (see #182).

Additionally:

- Some unit tests are not updated to test Bitcoin Static features, still trying.




