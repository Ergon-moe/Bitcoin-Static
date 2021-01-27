# Bitcoin Static Seeder v22.2.0

| Usage                                                                                     |
| :---------------------------------------------------------------------------------------- |
| `bitcoin-seeder -host=<host> -ns=<ns> [-mbox=<mbox>] [-threads=<threads>] [-port=<port>]` |

Options
-------

| Argument                | Description                                         |
| :---------------------- | :-------------------------------------------------- |
| `-?`, `-h`, `-help`     | Print this help message and exit                    |
| `-dnsthreads=<threads>` | Number of DNS server threads (default: 4)           |
| `-filter=<f1,f2,...>`   | Allow these flag combinations as filters            |
| `-host=<host>`          | Hostname of the DNS seed                            |
| `-mbox=<mbox>`          | E-Mail address reported in SOA records              |
| `-ns=<ns>`              | Hostname of the nameserver                          |
| `-threads=<threads>`    | Number of crawlers to run in parallel (default: 96) |
| `-version`              | Print version and exit                              |

Connection options
------------------

| Argument               | Description                             |
| :--------------------- | :-------------------------------------- |
| `-onion=<ip:port>`     | Tor proxy IP/Port                       |
| `-port=<port>`         | UDP port to listen on (default: 53)     |
| `-proxyipv4=<ip:port>` | IPV4 SOCKS5 proxy IP/Port               |
| `-proxyipv6=<ip:port>` | IPV6 SOCKS5 proxy IP/Port               |
| `-wipeban`             | Wipe list of banned nodes (default: 0)  |
| `-wipeignore`          | Wipe list of ignored nodes (default: 0) |

Chain selection options
-----------------------

| Argument    | Description                                                                                                                                                        |
| :---------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `-regtest`  | Enter regression test mode, which uses a special chain in which blocks can be solved instantly. This is intended for regression testing tools and app development. |
| `-scalenet` | Use the scaling test chain                                                                                                                                         |
| `-testnet`  | Use the test chain                                                                                                                                                 |
| `-testnet4` | Use the test4 chain                                                                                                                                                |
