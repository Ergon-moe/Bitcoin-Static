// Copyright (c) 2017-2019 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <seeder/bitcoin.h>

#include <hash.h>
#include <netbase.h>
#include <seeder/db.h>
#include <serialize.h>
#include <uint256.h>

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <ctime>

// The network magic to use.
CMessageHeader::MessageMagic netMagic = {{135, 215, 51, 46}};

#define BITCOIN_SEED_NONCE 0x0539a019ca550825ULL

static const uint32_t allones(-1);

void CSeederNode::BeginMessage(const char *pszCommand) {
    if (nHeaderStart != allones) {
        AbortMessage();
    }
    nHeaderStart = vSend.size();
    vSend << CMessageHeader(netMagic, pszCommand, 0);
    nMessageStart = vSend.size();
    // std::fprintf(stdout, "%s: SEND %s\n", ToString(you).c_str(), pszCommand);
}

void CSeederNode::AbortMessage() {
    if (nHeaderStart == allones) {
        return;
    }
    vSend.resize(nHeaderStart);
    nHeaderStart = allones;
    nMessageStart = allones;
}

void CSeederNode::EndMessage() {
    if (nHeaderStart == allones) {
        return;
    }
    uint32_t nSize = vSend.size() - nMessageStart;
    std::memcpy((char *)&vSend[nHeaderStart] +
                    offsetof(CMessageHeader, nMessageSize),
                &nSize, sizeof(nSize));
    if (vSend.GetVersion() >= 209) {
        uint256 hash = Hash(vSend.begin() + nMessageStart, vSend.end());
        unsigned int nChecksum = 0;
        std::memcpy(&nChecksum, &hash, sizeof(nChecksum));
        assert(nMessageStart - nHeaderStart >=
               offsetof(CMessageHeader, pchChecksum) + sizeof(nChecksum));
        std::memcpy((char *)&vSend[nHeaderStart] +
                        offsetof(CMessageHeader, pchChecksum),
                    &nChecksum, sizeof(nChecksum));
    }
    nHeaderStart = allones;
    nMessageStart = allones;
}

void CSeederNode::Send() {
    if (sock == INVALID_SOCKET) {
        return;
    }
    if (vSend.empty()) {
        return;
    }
    int nBytes = send(sock, &vSend[0], vSend.size(), 0);
    if (nBytes > 0) {
        vSend.erase(vSend.begin(), vSend.begin() + nBytes);
    } else {
        CloseSocket(sock);
    }
}

void CSeederNode::PushVersion() {
    const int64_t nTime = static_cast<int64_t>(std::time(nullptr)); // nTime sent as int64_t always
    const uint64_t nLocalNonce = BITCOIN_SEED_NONCE;
    const int64_t nLocalServices = 0;
    const CAddress me(CService{}, ServiceFlags(NODE_NETWORK | NODE_BITCOIN_CASH));
    BeginMessage("version");
    const int nBestHeight = GetRequireHeight();
    const std::string ver = "/bitcoin-cash-seeder:0.15/";
    vSend << PROTOCOL_VERSION << nLocalServices << nTime << you << me
          << nLocalNonce << ver << nBestHeight;
    EndMessage();
}

PeerMessagingState CSeederNode::ProcessMessage(const std::string &strCommand,
                                               CDataStream &recv) {
    // std::fprintf(stdout, "%s: RECV %s\n", ToString(you).c_str(),
    //              strCommand.c_str());
    if (strCommand == "version") {
        int64_t nTime;
        CAddress addrMe;
        CAddress addrFrom;
        uint64_t nNonce = 1;
        uint64_t nServiceInt;
        recv >> nVersion >> nServiceInt >> nTime >> addrMe;
        you.nServices = ServiceFlags(nServiceInt);
        recv >> addrFrom >> nNonce;
        recv >> strSubVer;
        recv >> nStartingHeight;

        BeginMessage("verack");
        EndMessage();
        vSend.SetVersion(std::min(nVersion, PROTOCOL_VERSION));
        return PeerMessagingState::AwaitingMessages;
    }

    if (strCommand == "verack") {
        vRecv.SetVersion(std::min(nVersion, PROTOCOL_VERSION));
        // std::fprintf(stdout, "\n%s: version %i\n", ToString(you).c_str(),
        //              nVersion);
        if (vAddr) {
            BeginMessage("getaddr");
            EndMessage();
            doneAfter = std::time(nullptr) + GetTimeout();
        } else {
            doneAfter = std::time(nullptr) + 1;
        }
        return PeerMessagingState::AwaitingMessages;
    }

    if (strCommand == "addr" && vAddr) {
        std::vector<CAddress> vAddrNew;
        recv >> vAddrNew;
        // std::fprintf(stdout, "%s: got %i addresses\n", ToString(you).c_str(),
        //              (int)vAddrNew.size());
        int64_t now = std::time(nullptr);
        std::vector<CAddress>::iterator it = vAddrNew.begin();
        if (vAddrNew.size() > 1) {
            if (doneAfter == 0 || doneAfter > now + 1) {
                doneAfter = now + 1;
            }
        }
        while (it != vAddrNew.end()) {
            CAddress &addr = *it;
            // std::fprintf(stdout, "%s: got address %s\n",
            //              ToString(you).c_str(),
            //              addr.ToString().c_str(), (int)(vAddr->size()));
            it++;
            if (addr.nTime <= 100000000 || addr.nTime > now + 600) {
                addr.nTime = now - 5 * 86400;
            }
            if (addr.nTime > now - 604800) {
                vAddr->push_back(addr);
            }
            // std::fprintf(stdout, "%s: added address %s (#%i)\n",
            //              ToString(you).c_str(),
            //              addr.ToString().c_str(), (int)(vAddr->size()));
            if (vAddr->size() > ADDR_SOFT_CAP) {
                doneAfter = 1;
                return PeerMessagingState::Finished;
            }
        }
        return PeerMessagingState::AwaitingMessages;
    }

    return PeerMessagingState::AwaitingMessages;
}

bool CSeederNode::ProcessMessages() {
    if (vRecv.empty()) {
        return false;
    }

    do {
        CDataStream::iterator pstart = std::search(
            vRecv.begin(), vRecv.end(), BEGIN(netMagic), END(netMagic));
        std::size_t nHeaderSize =
            GetSerializeSize(CMessageHeader(netMagic), vRecv.GetVersion());
        if (std::size_t(vRecv.end() - pstart) < nHeaderSize) {
            if (vRecv.size() > nHeaderSize) {
                vRecv.erase(vRecv.begin(), vRecv.end() - nHeaderSize);
            }
            break;
        }
        vRecv.erase(vRecv.begin(), pstart);
        std::vector<char> vHeaderSave(vRecv.begin(),
                                      vRecv.begin() + nHeaderSize);
        CMessageHeader hdr(netMagic);
        vRecv >> hdr;
        if (!hdr.IsValidWithoutConfig(netMagic)) {
            // std::fprintf(stdout, "%s: BAD (invalid header)\n",
            //              ToString(you).c_str());
            ban = 100000;
            return true;
        }
        std::string strCommand = hdr.GetCommand();
        unsigned int nMessageSize = hdr.nMessageSize;
        if (nMessageSize > MAX_SIZE) {
            // std::fprintf(stdout, "%s: BAD (message too large)\n",
            //              ToString(you).c_str());
            ban = 100000;
            return true;
        }
        if (nMessageSize > vRecv.size()) {
            vRecv.insert(vRecv.begin(), vHeaderSave.begin(), vHeaderSave.end());
            break;
        }
        if (vRecv.GetVersion() >= 209) {
            uint256 hash = Hash(vRecv.begin(), vRecv.begin() + nMessageSize);
            if (std::memcmp(hash.begin(), hdr.pchChecksum,
                            CMessageHeader::CHECKSUM_SIZE) != 0) {
                continue;
            }
        }
        CDataStream vMsg(vRecv.begin(), vRecv.begin() + nMessageSize,
                         vRecv.GetType(), vRecv.GetVersion());
        vRecv.ignore(nMessageSize);
        if (ProcessMessage(strCommand, vMsg) == PeerMessagingState::Finished) {
            return true;
        }
        // std::fprintf(stdout, "%s: done processing %s\n",
        //              ToString(you).c_str(),
        //              strCommand.c_str());
    } while (1);
    return false;
}

CSeederNode::CSeederNode(const CService &ip, std::vector<CAddress> *vAddrIn)
    : sock(INVALID_SOCKET), vSend(SER_NETWORK, 0), vRecv(SER_NETWORK, 0),
      nHeaderStart(-1), nMessageStart(-1), nVersion(0), nStartingHeight(0),
      vAddr(vAddrIn), ban(0), doneAfter(0),
      you(ip, ServiceFlags(NODE_NETWORK | NODE_BITCOIN_CASH)) {
    if (std::time(nullptr) > 1329696000) {
        vSend.SetVersion(209);
        vRecv.SetVersion(209);
    }
}

bool CSeederNode::Run() {
    // FIXME: This logic is duplicated with CConnman::ConnectNode for no
    // good reason.
    bool connected = false;
    proxyType proxy;

    if (you.IsValid()) {
        bool proxyConnectionFailed = false;

        if (GetProxy(you.GetNetwork(), proxy)) {
            sock = CreateSocket(proxy.proxy);
            if (sock == INVALID_SOCKET) {
                return false;
            }
            connected = ConnectThroughProxy(
                proxy, you.ToStringIP(), you.GetPort(), sock, nConnectTimeout,
                &proxyConnectionFailed);
        } else {
            // no proxy needed (none set for target network)
            sock = CreateSocket(you);
            if (sock == INVALID_SOCKET) {
                return false;
            }
            // no proxy needed (none set for target network)
            connected =
                ConnectSocketDirectly(you, sock, nConnectTimeout, false);
        }
    }

    if (!connected) {
        // std::fprintf(stdout, "Cannot connect to %s\n", ToString(you).c_str());
        CloseSocket(sock);
        return false;
    }

    PushVersion();
    Send();

    bool res = true;
    int64_t now;
    auto Predicate = [&now, this] {
        now = std::time(nullptr);
        return ban == 0 && (doneAfter == 0 || doneAfter > now) && sock != INVALID_SOCKET;
    };
    while (Predicate()) {
        char pchBuf[0x10000];
        fd_set fdsetRecv;
        fd_set fdsetError;
        FD_ZERO(&fdsetRecv);
        FD_ZERO(&fdsetError);
        FD_SET(sock, &fdsetRecv);
        FD_SET(sock, &fdsetError);
        struct timeval wa;
        if (doneAfter) {
            wa.tv_sec = doneAfter - now;
            wa.tv_usec = 0;
        } else {
            wa.tv_sec = GetTimeout();
            wa.tv_usec = 0;
        }
        int ret = select(sock + 1, &fdsetRecv, nullptr, &fdsetError, &wa);
        if (ret != 1) {
            if (!doneAfter) {
                res = false;
            }
            break;
        }
        int nBytes = recv(sock, pchBuf, sizeof(pchBuf), 0);
        int nPos = vRecv.size();
        if (nBytes > 0) {
            vRecv.resize(nPos + nBytes);
            std::memcpy(&vRecv[nPos], pchBuf, nBytes);
        } else if (nBytes == 0) {
            // std::fprintf(stdout, "%s: BAD (connection closed prematurely)\n",
            //              ToString(you).c_str());
            res = false;
            break;
        } else {
            // std::fprintf(stdout, "%s: BAD (connection error)\n",
            //              ToString(you).c_str());
            res = false;
            break;
        }
        ProcessMessages();
        Send();
    }
    if (sock == INVALID_SOCKET) {
        res = false;
    } else
        CloseSocket(sock);
    return (ban == 0) && res;
}

bool TestNode(const CService &cip, int &ban, int &clientV,
              std::string &clientSV, int &blocks,
              std::vector<CAddress> *vAddr, ServiceFlags &services) {
    try {
        CSeederNode node(cip, vAddr);
        bool ret = node.Run();
        if (!ret) {
            ban = node.GetBan();
        } else {
            ban = 0;
        }
        clientV = node.GetClientVersion();
        clientSV = node.GetClientSubVersion();
        blocks = node.GetStartingHeight();
        services = node.GetServices();
        // std::fprintf(stdout, "%s: %s!!!\n", cip.ToString().c_str(), ret ? "GOOD" :
        //              "BAD");
        return ret;
    } catch (std::ios_base::failure &e) {
        ban = 0;
        return false;
    }
}
