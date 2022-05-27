#!/usr/bin/env python3
"""Script to poll bitcoind for MTP to elapse and broadcast a transaction"""

import aiohttp
import asyncio
import base64
import hashlib
import json
import sys
import time

from typing import Union


def sha256d(data: bytes) -> bytes:
    for _ in range(2):
        h = hashlib.sha256()
        h.update(data)
        data = h.digest()
    return data


def get_time_str() -> str:
    return time.strftime('%a, %d %b %Y %H:%M:%S GMT', time.gmtime())


class BitcoinDInfo:
    __slots__ = ('host', 'port', 'rpcuser', 'rpcpass', 'idctr')

    def __init__(self, host, port, rpcuser, rpcpass):
        self.host, self.port, self.rpcuser, self.rpcpass = host, port, rpcuser, rpcpass
        self.idctr = 0

    @property
    def authtoken(self) -> str:
        return base64.b64encode(f'{self.rpcuser}:{self.rpcpass}'.encode('utf-8')).decode('ascii')

    @property
    def url(self) -> str:
        return f"http://{self.host}:{self.port}"

    @property
    def nextid(self) -> int:
        self.idctr += 1
        return self.idctr

    def __str__(self) -> str:
        return f"BitcoinD at {self.url}"

    def __repr__(self) -> str:
        return f"<{str(self)}>"


class BadStatus(RuntimeError):
    pass


class BitcoinDHelper:

    def __init__(self, info: BitcoinDInfo):
        self.info = info

    async def invoke_rpc_method(self, method: str, params: list = []) -> Union[dict, list, int, str, float, type(None)]:
        info = self.info
        #print(f"Connecting to {info} ...")
        req_json = {"jsonrpc": 2.0, "id": info.nextid, "method": method, "params": params}
        req_data = json.dumps(req_json).encode('utf-8')
        headers = {"Authorization": f"Basic {info.authtoken}"}
        async with aiohttp.ClientSession(headers=headers) as session:
            async with session.post(info.url, data=req_data) as r:
                if r.status != 200:
                    try:
                        j = await r.json()
                        error = j["error"]
                    except (KeyError, json.JSONDecodeError, aiohttp.ClientError):
                        error = await r.text()
                    err_text = f"status: {r.status} reason: {r.reason}"
                    raise BadStatus(err_text, error, info)
                else:
                    j = await r.json()
                    if j.get('error') is not None:
                        raise RuntimeError("got error response", j["error"], info)
                    else:
                        return j['result']

    async def getmediantime(self) -> int:
        res = await self.invoke_rpc_method("getblockchaininfo")
        return int(res["mediantime"])

    async def sendrawtransaction(self, txnhex, allowhighfees: bool = False) -> str:
        return await self.invoke_rpc_method("sendrawtransaction", [txnhex, allowhighfees])


async def poller(info: BitcoinDInfo, txnhex: str, mtptarget: int, sleeptime: float):
    bd = BitcoinDHelper(info)

    while True:
        print(f"[{get_time_str()}]", end=" ")
        try:
            mtp = await bd.getmediantime()
        except (OSError, ValueError, KeyError, BadStatus) as e:
            # bitcoind possibly temporarily down, restarted, restarting, etc. Just print a message and continue looping
            # after sleeping
            print(e)
        else:
            print("mediantime:", mtp, "", end="")
            if mtp < mtptarget:
                print(f"< {mtptarget}, sleeping {sleeptime:1.1f} seconds ...")
            else:
                print(f">= {mtptarget}, broadcasting txn of {len(bytes.fromhex(txnhex))} bytes ...")
                result = await bd.sendrawtransaction(txnhex)
                print("Response: ", result)
                expected_txid = sha256d(bytes.fromhex(txnhex))[::-1].hex()
                assert result == expected_txid
                print("Done! Exiting!")
                sys.exit(0)
        await asyncio.sleep(sleeptime)


def main():
    if len(sys.argv) not in (7, 8):
        sys.exit(f"Usage: {sys.argv[0]} <host> <port> <rpcuser> <rpcpass> <mtptarget> <txnhex> [sleeptime]\n"
                 """
    <host>      - The IP address or host of the bitcoind HTTP-RPC server (e.g. localhost)
    <port>      - The port for the bitcoind HTTP-RPC server (e.g. 8332)
    <rpcuser>   - The -rpcuser specified in bitcoin.conf for the RPC server
    <rpcpass>   - The -rpcpassword specified in bitcoin.conf for the RPC server
    <mtptarget> - The MTP time after which we wish to broadcast the transaction
    <txnhex>    - The hexadecimal encoded transaction to broadcast (as would be given to sendrawtransaction RPC)
    [sleeptime] - The amount of time to sleep between polls of the MTP time via getblockchaininfo. Default: 5.0.
""")
    info = BitcoinDInfo(*sys.argv[1:5])
    mtptarget = int(sys.argv[5])
    txnhex = sys.argv[6]
    assert bytes.fromhex(txnhex).hex() == txnhex
    sleeptime = 5.0
    if len(sys.argv) >= 8:
        sleeptime = float(sys.argv[7])

    asyncio.run(poller(info, txnhex, mtptarget, sleeptime))


if __name__ == '__main__':
    main()
