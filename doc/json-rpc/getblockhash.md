`getblockhash` JSON-RPC command
===============================

**`getblockhash height`**

```
Returns hash of block in best-block-chain at height provided.
```

Arguments
---------

```
1. height         (numeric, required) The height index
```

Result
------

```
"hash"         (string) The block hash
```

Examples
--------

```
> bitcoin-cli getblockhash 1000
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getblockhash", "params": [1000] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/
```

***

*Bitcoin Static Daemon version v22.2.0*
