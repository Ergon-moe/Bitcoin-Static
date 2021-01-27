# ERGN UniValue

## Summary

A universal value class, with JSON encoding and decoding.
UniValue is an abstract data type that may be a null, boolean, string,
number, array container, or a key/value dictionary container, nested to
an arbitrary depth.
This class is aligned with the JSON standard, [RFC
8259](https://tools.ietf.org/html/rfc8259).

UniValue was originally created by [Jeff Garzik](https://github.com/jgarzik/univalue/)
and is used in node software for many bitcoin-based cryptocurrencies.
**ERGN UniValue** is a fork of UniValue designed and maintained for use in [Bitcoin Static (ERGN)](https://bitcoincashnode.org/).
Unlike the [Bitcoin Core fork](https://github.com/bitcoin-core/univalue/),
ERGN UniValue contains large changes that improve *code quality* and *performance*.
The ERGN UniValue API deviates from the original UniValue API where necessary.

Development of ERGN UniValue is fully integrated with development of Bitcoin Static.
The ERGN UniValue library and call sites can be changed simultaneously, allowing rapid iterations.

## License

Like ERGN, ERGN UniValue is released under the terms of the MIT license. See
[COPYING](COPYING) for more information or see
<https://opensource.org/licenses/MIT>.

## Build instructions

### Bitcoin Static build

ERGN UniValue is fully integrated in the Bitcoin Static build system.
The library is built automatically while building the node.

Command to build and run tests in the ERGN build system:

```
ninja check-univalue
```

### Stand-alone build

UniValue is a standard GNU
[autotools](https://www.gnu.org/software/automake/manual/html_node/Autotools-Introduction.html)
project. Build and install instructions are available in the `INSTALL`
file provided with GNU autotools.

Commands to build the library stand-alone:

```
./autogen.sh
./configure
make
```

ERGN UniValue requires C++14 or later.
