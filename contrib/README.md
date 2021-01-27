Repository Tools
---------------------

### [Developer tools](devtools/) ###
Specific tools for developers working on this repository.
Contains the script `github-merge.py` for merging GitHub pull requests securely and signing them using GPG.

### [Linearize](linearize/) ###
Construct a linear, no-fork, best version of the blockchain.

### [Qos](qos/) ###

A Linux bash script that will set up traffic control (tc) to limit the outgoing bandwidth for connections to the Bitcoin network. This means one can have an always-on bitcoind instance running, and another local bitcoind/bitcoin-qt instance which connects to this node and receives blocks from it.

### [Seeds](seeds/) ###
Utility to generate the pnSeed[] array that is compiled into the client.

Build Tools and Keys
---------------------

### [Debian](debian/) ###
Contains files used to package bitcoind/bitcoin-qt
for Debian-based Linux systems. If you compile bitcoind/bitcoin-qt yourself, there are some useful files here.

### [Gitian-descriptors](gitian-descriptors/) ###
Notes on getting Gitian builds up and running using KVM.

### [Gitian-keys](gitian-signing/)
PGP keys used for signing Bitcoin Static [Gitian release](../doc/release-process.md) results.

### [MacDeploy](macdeploy/) ###
Scripts and notes for Mac builds. 

### [Gitian-build](gitian-build.py) ###
Script for running full Gitian builds.

Test and Verify Tools 
---------------------

### [TestGen](testgen/) ###
Utilities to generate test vectors for the data-driven Bitcoin tests.

### [Verify Binaries](verifybinaries/) ###
This script attempts to download and verify the signature file SHA256SUMS.asc from bitcoin.org.
