`getaddednodeinfo` JSON-RPC command
===================================

**`getaddednodeinfo ( "node" )`**

```
Returns information about the given added node, or all added nodes
(note that onetry addnodes are not listed here)
```

Arguments
---------

```
1. "node"   (string, optional) If provided, return information about this specific node, otherwise all nodes are returned.
```

Result
------

```
[
  {
    "addednode" : "192.168.0.201",   (string) The node IP address or name (as provided to addnode)
    "connected" : true|false,          (boolean) If connected
    "addresses" : [                    (list of objects) Only when connected = true
       {
         "address" : "192.168.0.201:8333",  (string) The bitcoin server IP and port we're connected to
         "connected" : "outbound"           (string) connection, inbound or outbound
       }
     ]
  }
  ,...
]
```

Examples
--------

```
> bitcoin-cli getaddednodeinfo "192.168.0.201"
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "getaddednodeinfo", "params": ["192.168.0.201"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/
```

***

*Bitcoin Static Daemon version v22.2.0*
