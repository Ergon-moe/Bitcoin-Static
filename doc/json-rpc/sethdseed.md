`sethdseed` JSON-RPC command
============================

**`sethdseed ( newkeypool "seed" )`**

```
Set or generate a new HD wallet seed. Non-HD wallets will not be upgraded to being a HD wallet. Wallets that are already
HD will have a new HD seed set so that new keys added to the keypool will be derived from this new seed.

Note that you will need to MAKE A NEW BACKUP of your wallet after setting the HD wallet seed.
```

Arguments
---------

```
1. "newkeypool"         (boolean, optional, default=true) Whether to flush old unused addresses, including change addresses, from the keypool and regenerate it.
                             If true, the next address from getnewaddress and change address from getrawchangeaddress will be from this new seed.
                             If false, addresses (including change addresses if the wallet already had HD Chain Split enabled) from the existing
                             keypool will be used until it has been depleted.
2. "seed"               (string, optional) The WIF private key to use as the new HD seed; if not provided a random seed will be used.
                             The seed value can be retrieved using the dumpwallet command. It is the private key marked hdseed=1
```

Examples
--------

```
> bitcoin-cli sethdseed
> bitcoin-cli sethdseed false
> bitcoin-cli sethdseed true "wifkey"
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "sethdseed", "params": [true, "wifkey"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/
```

***

*Bitcoin Static Daemon version v22.2.0*
