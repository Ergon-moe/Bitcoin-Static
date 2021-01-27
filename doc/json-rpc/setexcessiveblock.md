`setexcessiveblock` JSON-RPC command
====================================

**`setexcessiveblock maxBlockSize`**

```
Set the excessive block size. Excessive blocks will not be used in the active chain or relayed. This discourages the propagation of blocks that you consider excessively large.
Arguments
1. maxBlockSize  (integer, required) Excessive block size in bytes.  Must be greater than 1000000.

Result
  maxBlockSize (integer) excessive block size in bytes
```

Examples
--------

```
> bitcoin-cli setexcessiveblock 128000000
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "setexcessiveblock", "params": [128000000] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/
```

***

*Bitcoin Static Daemon version v22.2.0*
