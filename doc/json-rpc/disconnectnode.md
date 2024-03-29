`disconnectnode` JSON-RPC command
=================================

**`disconnectnode ( "address" nodeid )`**

```
Immediately disconnects from the specified peer node.

Strictly one out of 'address' and 'nodeid' can be provided to identify the node.

To disconnect by nodeid, either set 'address' to the empty string, or call using the named 'nodeid' argument only.
```

Arguments
---------

```
1. "address"     (string, optional) The IP address/port of the node
2. "nodeid"      (number, optional) The node ID (see getpeerinfo for node IDs)
```

Examples
--------

```
> bitcoin-cli disconnectnode "192.168.0.6:8333"
> bitcoin-cli disconnectnode "" 1
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "disconnectnode", "params": ["192.168.0.6:8333"] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/
> curl --user myusername --data-binary '{"jsonrpc": "1.0", "id":"curltest", "method": "disconnectnode", "params": ["", 1] }' -H 'content-type: text/plain;' http://127.0.0.1:8332/
```

***

*Bitcoin Static Daemon version v22.2.0*
