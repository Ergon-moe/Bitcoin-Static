`sendmany` JSON-RPC command
===========================

**`sendmany "" {"address":amount} ( minconf "comment" ["address",...] )`**

```
Send multiple times. Amounts are double-precision floating point numbers.
```

Arguments
---------

```
1. "dummy"               (string, required) Must be set to "" for backwards compatibility.
2. "amounts"             (string, required) A json object with addresses and amounts
    {
      "address":amount   (numeric or string) The bitcoin address is the key, the numeric amount (can be string) in XRG is the value
      ,...
    }
3. minconf                 (numeric, optional, default=1) Only use the balance confirmed at least this many times.
4. "comment"             (string, optional) A comment
5. subtractfeefrom         (array, optional) A json array with addresses.
                           The fee will be equally deducted from the amount of each selected address.
                           Those recipients will receive less bitcoins than you enter in their corresponding amount field.
                           If no addresses are specified here, the sender pays the fee.
    [
      "address"          (string) Subtract fee from this address
      ,...
    ]
```

Result
------

```
"txid"                   (string) The transaction id for the send. Only 1 transaction is created regardless of
                                    the number of addresses.
```

Examples
--------

Send two amounts to two different addresses
-------------------------------------------

```
> bitcoin-cli sendmany "" "{\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\":0.01,\"1353tsE8YMTA4EuV7dgUXGjNFf9KpVvKHz\":0.02}"
```

Send two amounts to two different addresses setting the confirmation and comment
--------------------------------------------------------------------------------

```
> bitcoin-cli sendmany "" "{\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\":0.01,\"1353tsE8YMTA4EuV7dgUXGjNFf9KpVvKHz\":0.02}" 6 "testing"
```

Send two amounts to two different addresses, subtract fee from amount
---------------------------------------------------------------------

```
> bitcoin-cli sendmany "" "{\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\":0.01,\"1353tsE8YMTA4EuV7dgUXGjNFf9KpVvKHz\":0.02}" 1 "" "[\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\",\"1353tsE8YMTA4EuV7dgUXGjNFf9KpVvKHz\"]"

As a JSON-RPC call
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "sendmany", "params": ["", "{\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\":0.01,\"1353tsE8YMTA4EuV7dgUXGjNFf9KpVvKHz\":0.02}", 6, "testing"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/
```

***

*Bitcoin Static Daemon version v22.2.0*
