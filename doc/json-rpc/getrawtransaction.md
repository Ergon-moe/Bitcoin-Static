`getrawtransaction` JSON-RPC command
====================================

**`getrawtransaction "txid" ( verbose "blockhash" )`**

```
NOTE: By default this function only works for mempool transactions. If the -txindex option is
enabled, it also works for blockchain transactions. If the block which contains the transaction
is known, its hash can be provided even for nodes without -txindex. Note that if a blockhash is
provided, only that block will be searched and if the transaction is in the mempool or other
blocks, or if this node does not have the given block available, the transaction will not be found.
DEPRECATED: for now, it also works for transactions with unspent outputs.

Return the raw transaction data.

If verbose is 'true', returns an Object with information about 'txid'.
If verbose is 'false' or omitted, returns a string that is serialized, hex-encoded data for 'txid'.
```

Arguments
---------

```
1. "txid"      (string, required) The transaction id
2. verbose     (bool, optional, default=false) If false, return a string, otherwise return a json object
3. "blockhash" (string, optional) The block in which to look for the transaction
```

Result (if verbose is not set or set to false)
----------------------------------------------

```
"data"      (string) The serialized, hex-encoded data for 'txid'
```

Result (if verbose is set to true)
----------------------------------

```
{
  "in_active_chain": b, (bool) Whether specified block is in the active chain or not (only present with explicit "blockhash" argument)
  "hex" : "data",       (string) The serialized, hex-encoded data for 'txid'
  "txid" : "id",        (string) The transaction id (same as provided)
  "hash" : "id",        (string) The transaction hash
  "size" : n,             (numeric) The serialized transaction size
  "version" : n,          (numeric) The version
  "locktime" : ttt,       (numeric) The lock time
  "vin" : [               (array of json objects)
     {
       "txid": "id",    (string) The transaction id
       "vout": n,         (numeric)
       "scriptSig": {     (json object) The script
         "asm": "asm",  (string) asm
         "hex": "hex"   (string) hex
       },
       "sequence": n      (numeric) The script sequence number
     }
     ,...
  ],
  "vout" : [              (array of json objects)
     {
       "value" : x.xxx,            (numeric) The value in XRG
       "n" : n,                    (numeric) index
       "scriptPubKey" : {          (json object)
         "asm" : "asm",          (string) the asm
         "hex" : "hex",          (string) the hex
         "reqSigs" : n,            (numeric) The required sigs
         "type" : "pubkeyhash",  (string) The type, eg 'pubkeyhash'
         "addresses" : [           (json array of string)
           "address"        (string) bitcoin address
           ,...
         ]
       }
     }
     ,...
  ],
  "blockhash" : "hash",   (string) the block hash
  "confirmations" : n,      (numeric) The confirmations
  "time" : ttt,             (numeric) The transaction time in seconds since epoch (Jan 1 1970 GMT)
  "blocktime" : ttt         (numeric) The block time in seconds since epoch (Jan 1 1970 GMT)
}
```

Examples
--------

```
> bitcoin-cli getrawtransaction "mytxid"
> bitcoin-cli getrawtransaction "mytxid" true
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getrawtransaction", "params": ["mytxid", true] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/
> bitcoin-cli getrawtransaction "mytxid" false "myblockhash"
> bitcoin-cli getrawtransaction "mytxid" true "myblockhash"
```

***

*Bitcoin Static Daemon version v22.2.0*
