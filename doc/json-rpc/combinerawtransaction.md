`combinerawtransaction` JSON-RPC command
========================================

**`combinerawtransaction ["hexstring",...]`**

```
Combine multiple partially signed transactions into one transaction.
The combined transaction may be another partially signed transaction or a
fully signed transaction.
Arguments:
1. "txs"         (string) A json array of hex strings of partially signed transactions
    [
      "hexstring"     (string) A transaction hash
      ,...
    ]
```

Result
------

```
"hex"            (string) The hex-encoded raw transaction with signature(s)
```

Examples
--------

```
> bitcoin-cli combinerawtransaction ["myhex1", "myhex2", "myhex3"]
```

***

*Bitcoin Static Daemon version v22.2.0*
