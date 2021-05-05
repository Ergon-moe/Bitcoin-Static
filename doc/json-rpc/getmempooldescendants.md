`getmempooldescendants` JSON-RPC command
========================================

**`getmempooldescendants "txid" ( verbose )`**

```
If txid is in the mempool, returns all in-mempool descendants.
```

Arguments
---------

```
1. "txid"                 (string, required) The transaction id (must be in mempool)
2. verbose                  (boolean, optional, default=false) True for a json object, false for array of transaction ids
```

Result (for verbose = false)
----------------------------

```
[                       (json array of strings)
  "transactionid"           (string) The transaction id of an in-mempool descendant transaction
  ,...
]
```

Result (for verbose = true)
---------------------------

```
{                           (json object)
  "transactionid" : {       (json object)
    "size" : n,             (numeric) transaction size.
    "fee" : n,              (numeric) transaction fee in XRG(DEPRECATED)
    "modifiedfee" : n,      (numeric) transaction fee with fee deltas used for mining priority (DEPRECATED)
    "time" : n,             (numeric) local time transaction entered pool in seconds since 1 Jan 1970 GMT
    "height" : n,           (numeric) block height when transaction entered pool
    "descendantcount" : n,  (numeric) number of in-mempool descendant transactions (including this one)
    "descendantsize" : n,   (numeric) transaction size of in-mempool descendants (including this one)
    "descendantfees" : n,   (numeric) modified fees (see above) of in-mempool descendants (including this one) (DEPRECATED)
    "ancestorcount" : n,    (numeric) number of in-mempool ancestor transactions (including this one)
    "ancestorsize" : n,     (numeric) transaction size of in-mempool ancestors (including this one)
    "ancestorfees" : n,     (numeric) modified fees (see above) of in-mempool ancestors (including this one) (DEPRECATED)
    "fees" : {
        "base" : n,         (numeric) transaction fee in XRG
        "modified" : n,     (numeric) transaction fee with fee deltas used for mining priority in XRG
        "ancestor" : n,     (numeric) modified fees (see above) of in-mempool ancestors (including this one) in XRG
        "descendant" : n,   (numeric) modified fees (see above) of in-mempool descendants (including this one) in XRG
    }
    "depends" : [           (array) unconfirmed transactions used as inputs for this transaction
        "transactionid",    (string) parent transaction id
       ... ]
    "spentby" : [           (array) unconfirmed transactions spending outputs from this transaction
        "transactionid",    (string) child transaction id
       ... ]
  }, ...
}
```

Examples
--------

```
> bitcoin-cli getmempooldescendants "mytxid"
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getmempooldescendants", "params": ["mytxid"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/
```

***

*Bitcoin Static Daemon version v22.2.0*
