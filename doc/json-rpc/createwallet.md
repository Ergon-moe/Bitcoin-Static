`createwallet` JSON-RPC command
===============================

**`createwallet "wallet_name" ( disable_private_keys )`**

```
Creates and loads a new wallet.
```

Arguments
---------

```
1. "wallet_name"    (string, required) The name for the new wallet. If this is a path, the wallet will be created at the path location.
2. disable_private_keys   (boolean, optional, default: false) Disable the possibility of private keys (only watchonlys are possible in this mode).
3. blank   (boolean, optional, default: false) Create a blank wallet. A blank wallet has no keys or HD seed. One can be set using sethdseed.
```

Result
------

```
{
  "name" :    <wallet_name>,        (string) The wallet name if created successfully. If the wallet was created using a full path, the wallet_name will be the full path.
  "warning" : <warning>,            (string) Warning message if wallet was not loaded cleanly.
}
```

Examples
--------

```
> bitcoin-cli createwallet "testwallet"
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "createwallet", "params": ["testwallet"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/
```

***

*Bitcoin Static Daemon version v22.2.0*
