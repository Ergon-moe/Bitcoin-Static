// Copyright (c) 2012-2016 The Bitcoin Core developers
// Copyright (c) 2020 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <rpc/client.h>
#include <rpc/server.h>
#include <rpc/util.h>

#include <config.h>
#include <core_io.h>
#include <init.h>
#include <interfaces/chain.h>
#include <key_io.h>
#include <netbase.h>

#include <test/setup_common.h>

#include <boost/algorithm/string.hpp>
#include <boost/test/unit_test.hpp>

#include <univalue.h>

#include <rpc/blockchain.h>

UniValue CallRPC(const std::string &args)
{
    std::vector<std::string> vArgs;
    boost::split(vArgs, args, boost::is_any_of(" \t"));
    std::string strMethod = vArgs[0];
    vArgs.erase(vArgs.begin());
    GlobalConfig config;
    JSONRPCRequest request;
    request.strMethod = strMethod;
    request.params = RPCConvertValues(strMethod, vArgs);
    request.fHelp = false;
    BOOST_CHECK(tableRPC[strMethod]);
    try {
        return tableRPC[strMethod]->call(config, request);
    } catch (const JSONRPCError &error) {
        throw std::runtime_error(error.message);
    }
}

BOOST_FIXTURE_TEST_SUITE(rpc_tests, TestingSetup)

BOOST_AUTO_TEST_CASE(rpc_rawparams) {
    // Test raw transaction API argument handling
    UniValue r;

    BOOST_CHECK_THROW(CallRPC("getrawtransaction"), std::runtime_error);
    BOOST_CHECK_THROW(CallRPC("getrawtransaction not_hex"), std::runtime_error);
    BOOST_CHECK_THROW(CallRPC("getrawtransaction "
                              "a3b807410df0b60fcb9736768df5823938b2f838694939ba"
                              "45f3c0a1bff150ed not_int"),
                      std::runtime_error);

    BOOST_CHECK_THROW(CallRPC("createrawtransaction"), std::runtime_error);
    BOOST_CHECK_THROW(CallRPC("createrawtransaction null null"),
                      std::runtime_error);
    BOOST_CHECK_THROW(CallRPC("createrawtransaction not_array"),
                      std::runtime_error);
    BOOST_CHECK_THROW(CallRPC("createrawtransaction {} {}"),
                      std::runtime_error);
    BOOST_CHECK_NO_THROW(CallRPC("createrawtransaction [] {}"));
    BOOST_CHECK_THROW(CallRPC("createrawtransaction [] {} extra"),
                      std::runtime_error);

    BOOST_CHECK_THROW(CallRPC("decoderawtransaction"), std::runtime_error);
    BOOST_CHECK_THROW(CallRPC("decoderawtransaction null"), std::runtime_error);
    BOOST_CHECK_THROW(CallRPC("decoderawtransaction DEADBEEF"),
                      std::runtime_error);
    std::string rawtx =
        "0100000001a15d57094aa7a21a28cb20b59aab8fc7d1149a3bdbcddba9c622e4f5f6a9"
        "9ece010000006c493046022100f93bb0e7d8db7bd46e40132d1f8242026e045f03a0ef"
        "e71bbb8e3f475e970d790221009337cd7f1f929f00cc6ff01f03729b069a7c21b59b17"
        "36ddfee5db5946c5da8c0121033b9b137ee87d5a812d6f506efdd37f0affa7ffc31071"
        "1c06c7f3e097c9447c52ffffffff0100e1f505000000001976a9140389035a9225b383"
        "9e2bbf32d826a1e222031fd888ac00000000";
    BOOST_CHECK_NO_THROW(
        r = CallRPC(std::string("decoderawtransaction ") + rawtx));
    BOOST_CHECK_EQUAL(r.get_obj()["size"].get_int(), 193);
    BOOST_CHECK_EQUAL(r.get_obj()["version"].get_int(), 1);
    BOOST_CHECK_EQUAL(r.get_obj()["locktime"].get_int(), 0);
    BOOST_CHECK_THROW(
        r = CallRPC(std::string("decoderawtransaction ") + rawtx + " extra"),
        std::runtime_error);

    // Only check failure cases for sendrawtransaction, there's no network to
    // send to...
    BOOST_CHECK_THROW(CallRPC("sendrawtransaction"), std::runtime_error);
    BOOST_CHECK_THROW(CallRPC("sendrawtransaction null"), std::runtime_error);
    BOOST_CHECK_THROW(CallRPC("sendrawtransaction DEADBEEF"),
                      std::runtime_error);
    BOOST_CHECK_THROW(
        CallRPC(std::string("sendrawtransaction ") + rawtx + " extra"),
        std::runtime_error);
}

BOOST_AUTO_TEST_CASE(rpc_togglenetwork) {
    UniValue r;

    r = CallRPC("getnetworkinfo");
    bool netState = r.get_obj()["networkactive"].get_bool();
    BOOST_CHECK_EQUAL(netState, true);

    BOOST_CHECK_NO_THROW(CallRPC("setnetworkactive false"));
    r = CallRPC("getnetworkinfo");
    int numConnection = r.get_obj()["connections"].get_int();
    BOOST_CHECK_EQUAL(numConnection, 0);

    netState = r.get_obj()["networkactive"].get_bool();
    BOOST_CHECK_EQUAL(netState, false);

    BOOST_CHECK_NO_THROW(CallRPC("setnetworkactive true"));
    r = CallRPC("getnetworkinfo");
    netState = r.get_obj()["networkactive"].get_bool();
    BOOST_CHECK_EQUAL(netState, true);
}

BOOST_AUTO_TEST_CASE(rpc_rawsign) {
    UniValue r;
    // input is a 1-of-2 multisig (so is output):
    std::string prevout = "[{\"txid\":"
                          "\"b4cc287e58f87cdae59417329f710f3ecd75a4ee1d2872b724"
                          "8f50977c8493f3\","
                          "\"vout\":1,\"scriptPubKey\":"
                          "\"a914b10c9df5f7edf436c697f02f1efdba4cf399615187\","
                          "\"amount\":3.14159,"
                          "\"redeemScript\":"
                          "\"512103debedc17b3df2badbcdd86d5feb4562b86fe182e5998"
                          "abd8bcd4f122c6155b1b21027e940bb73ab8732bfdf7f9216ece"
                          "fca5b94d6df834e77e108f68e66f126044c052ae\"}]";
    r = CallRPC(std::string("createrawtransaction ") + prevout + " " +
                "{\"3HqAe9LtNBjnsfM4CyYaWTnvCaUYT7v4oZ\":11}");
    std::string notsigned = r.get_str();
    std::string privkey1 =
        "\"KzsXybp9jX64P5ekX1KUxRQ79Jht9uzW7LorgwE65i5rWACL6LQe\"";
    std::string privkey2 =
        "\"Kyhdf5LuKTRx4ge69ybABsiUAWjVRK4XGxAKk2FQLp2HjGMy87Z4\"";
    InitInterfaces interfaces;
    interfaces.chain = interfaces::MakeChain();
    g_rpc_interfaces = &interfaces;
    r = CallRPC(std::string("signrawtransactionwithkey ") + notsigned + " [] " +
                prevout);
    BOOST_CHECK_EQUAL(r.get_obj()["complete"].get_bool(), false);
    r = CallRPC(std::string("signrawtransactionwithkey ") + notsigned + " [" +
                privkey1 + "," + privkey2 + "] " + prevout);
    BOOST_CHECK_EQUAL(r.get_obj()["complete"].get_bool(), true);
    g_rpc_interfaces = nullptr;
}

BOOST_AUTO_TEST_CASE(rpc_rawsign_missing_amount) {
    // Old format, missing amount parameter for prevout should generate
    // an RPC error.  This is because of new replay-protected tx's require
    // nonzero amount present in signed tx.
    // See: https://github.com/Bitcoin-ABC/bitcoin-abc/issues/63
    // (We will re-use the tx + keys from the above rpc_rawsign test for
    // simplicity.)
    UniValue r;
    std::string prevout = "[{\"txid\":"
                          "\"b4cc287e58f87cdae59417329f710f3ecd75a4ee1d2872b724"
                          "8f50977c8493f3\","
                          "\"vout\":1,\"scriptPubKey\":"
                          "\"a914b10c9df5f7edf436c697f02f1efdba4cf399615187\","
                          "\"redeemScript\":"
                          "\"512103debedc17b3df2badbcdd86d5feb4562b86fe182e5998"
                          "abd8bcd4f122c6155b1b21027e940bb73ab8732bfdf7f9216ece"
                          "fca5b94d6df834e77e108f68e66f126044c052ae\"}]";
    r = CallRPC(std::string("createrawtransaction ") + prevout + " " +
                "{\"3HqAe9LtNBjnsfM4CyYaWTnvCaUYT7v4oZ\":11}");
    std::string notsigned = r.get_str();
    std::string privkey1 =
        "\"KzsXybp9jX64P5ekX1KUxRQ79Jht9uzW7LorgwE65i5rWACL6LQe\"";
    std::string privkey2 =
        "\"Kyhdf5LuKTRx4ge69ybABsiUAWjVRK4XGxAKk2FQLp2HjGMy87Z4\"";

    bool exceptionThrownDueToMissingAmount = false,
         errorWasMissingAmount = false;

    InitInterfaces interfaces;
    interfaces.chain = interfaces::MakeChain();
    g_rpc_interfaces = &interfaces;

    try {
        r = CallRPC(std::string("signrawtransactionwithkey ") + notsigned +
                    " [" + privkey1 + "," + privkey2 + "] " + prevout);
    } catch (const std::runtime_error &e) {
        exceptionThrownDueToMissingAmount = true;
        if (std::string(e.what()).find("amount") != std::string::npos) {
            errorWasMissingAmount = true;
        }
    }
    BOOST_CHECK(exceptionThrownDueToMissingAmount == true);
    BOOST_CHECK(errorWasMissingAmount == true);

    g_rpc_interfaces = nullptr;
}

BOOST_AUTO_TEST_CASE(rpc_createraw_op_return) {
    BOOST_CHECK_NO_THROW(
        CallRPC("createrawtransaction "
                "[{\"txid\":"
                "\"a3b807410df0b60fcb9736768df5823938b2f838694939ba45f3c0a1bff1"
                "50ed\",\"vout\":0}] {\"data\":\"68656c6c6f776f726c64\"}"));

    // Allow more than one data transaction output
    BOOST_CHECK_NO_THROW(CallRPC("createrawtransaction "
                                 "[{\"txid\":"
                                 "\"a3b807410df0b60fcb9736768df5823938b2f838694"
                                 "939ba45f3c0a1bff150ed\",\"vout\":0}] "
                                 "{\"data\":\"68656c6c6f776f726c64\",\"data\":"
                                 "\"68656c6c6f776f726c64\"}"));

    // Key not "data" (bad address)
    BOOST_CHECK_THROW(
        CallRPC("createrawtransaction "
                "[{\"txid\":"
                "\"a3b807410df0b60fcb9736768df5823938b2f838694939ba45f3c0a1bff1"
                "50ed\",\"vout\":0}] {\"somedata\":\"68656c6c6f776f726c64\"}"),
        std::runtime_error);

    // Bad hex encoding of data output
    BOOST_CHECK_THROW(
        CallRPC("createrawtransaction "
                "[{\"txid\":"
                "\"a3b807410df0b60fcb9736768df5823938b2f838694939ba45f3c0a1bff1"
                "50ed\",\"vout\":0}] {\"data\":\"12345\"}"),
        std::runtime_error);
    BOOST_CHECK_THROW(
        CallRPC("createrawtransaction "
                "[{\"txid\":"
                "\"a3b807410df0b60fcb9736768df5823938b2f838694939ba45f3c0a1bff1"
                "50ed\",\"vout\":0}] {\"data\":\"12345g\"}"),
        std::runtime_error);

    // Data 81 bytes long
    BOOST_CHECK_NO_THROW(
        CallRPC("createrawtransaction "
                "[{\"txid\":"
                "\"a3b807410df0b60fcb9736768df5823938b2f838694939ba45f3c0a1bff1"
                "50ed\",\"vout\":0}] "
                "{\"data\":"
                "\"010203040506070809101112131415161718192021222324252627282930"
                "31323334353637383940414243444546474849505152535455565758596061"
                "6263646566676869707172737475767778798081\"}"));
}

BOOST_AUTO_TEST_CASE(rpc_format_monetary_values) {
    BOOST_CHECK(UniValue::stringify(ValueFromAmount(Amount::zero())) == "0.00000000");
    BOOST_CHECK(UniValue::stringify(ValueFromAmount(FIXOSHI)) == "0.00000001");
    BOOST_CHECK(UniValue::stringify(ValueFromAmount(17622195 * FIXOSHI)) == "0.17622195");
    BOOST_CHECK(UniValue::stringify(ValueFromAmount(50000000 * FIXOSHI)) == "0.50000000");
    BOOST_CHECK(UniValue::stringify(ValueFromAmount(89898989 * FIXOSHI)) == "0.89898989");
    BOOST_CHECK(UniValue::stringify(ValueFromAmount(100000000 * FIXOSHI)) == "1.00000000");
    BOOST_CHECK(UniValue::stringify(ValueFromAmount(int64_t(2099999999999990) * FIXOSHI)) == "20999999.99999990");
    BOOST_CHECK(UniValue::stringify(ValueFromAmount(int64_t(2099999999999999) * FIXOSHI)) == "20999999.99999999");

    BOOST_CHECK_EQUAL(UniValue::stringify(ValueFromAmount(Amount::zero())), "0.00000000");
    BOOST_CHECK_EQUAL(UniValue::stringify(ValueFromAmount(123456789 * (COIN / 10000))), "12345.67890000");
    BOOST_CHECK_EQUAL(UniValue::stringify(ValueFromAmount(-1 * COIN)), "-1.00000000");
    BOOST_CHECK_EQUAL(UniValue::stringify(ValueFromAmount(-1 * COIN / 10)), "-0.10000000");

    BOOST_CHECK_EQUAL(UniValue::stringify(ValueFromAmount(100000000 * COIN)), "100000000.00000000");
    BOOST_CHECK_EQUAL(UniValue::stringify(ValueFromAmount(10000000 * COIN)), "10000000.00000000");
    BOOST_CHECK_EQUAL(UniValue::stringify(ValueFromAmount(1000000 * COIN)), "1000000.00000000");
    BOOST_CHECK_EQUAL(UniValue::stringify(ValueFromAmount(100000 * COIN)), "100000.00000000");
    BOOST_CHECK_EQUAL(UniValue::stringify(ValueFromAmount(10000 * COIN)), "10000.00000000");
    BOOST_CHECK_EQUAL(UniValue::stringify(ValueFromAmount(1000 * COIN)), "1000.00000000");
    BOOST_CHECK_EQUAL(UniValue::stringify(ValueFromAmount(100 * COIN)), "100.00000000");
    BOOST_CHECK_EQUAL(UniValue::stringify(ValueFromAmount(10 * COIN)), "10.00000000");
    BOOST_CHECK_EQUAL(UniValue::stringify(ValueFromAmount(COIN)), "1.00000000");
    BOOST_CHECK_EQUAL(UniValue::stringify(ValueFromAmount(COIN / 10)), "0.10000000");
    BOOST_CHECK_EQUAL(UniValue::stringify(ValueFromAmount(COIN / 100)), "0.01000000");
    BOOST_CHECK_EQUAL(UniValue::stringify(ValueFromAmount(COIN / 1000)), "0.00100000");
    BOOST_CHECK_EQUAL(UniValue::stringify(ValueFromAmount(COIN / 10000)), "0.00010000");
    BOOST_CHECK_EQUAL(UniValue::stringify(ValueFromAmount(COIN / 100000)), "0.00001000");
    BOOST_CHECK_EQUAL(UniValue::stringify(ValueFromAmount(COIN / 1000000)), "0.00000100");
    BOOST_CHECK_EQUAL(UniValue::stringify(ValueFromAmount(COIN / 10000000)), "0.00000010");
    BOOST_CHECK_EQUAL(UniValue::stringify(ValueFromAmount(COIN / 100000000)), "0.00000001");
}

static UniValue ValueFromString(const std::string &str) {
    UniValue value;
    value.setNumStr(str);
    BOOST_CHECK(value.isNum());
    return value;
}

BOOST_AUTO_TEST_CASE(rpc_parse_monetary_values) {
    BOOST_CHECK_THROW(AmountFromValue(ValueFromString("-0.00000001")), JSONRPCError);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0")), Amount::zero());
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.00000000")),
                      Amount::zero());
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.00000001")), FIXOSHI);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.17622195")),
                      17622195 * FIXOSHI);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.5")),
                      50000000 * FIXOSHI);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.50000000")),
                      50000000 * FIXOSHI);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.89898989")),
                      89898989 * FIXOSHI);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("1.00000000")),
                      100000000 * FIXOSHI);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("20999999.9999999")),
                      int64_t(2099999999999990) * FIXOSHI);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("20999999.99999999")),
                      int64_t(2099999999999999) * FIXOSHI);

    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("1e-8")),
                      COIN / 100000000);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.1e-7")),
                      COIN / 100000000);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.01e-6")),
                      COIN / 100000000);
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString(
                          "0."
                          "0000000000000000000000000000000000000000000000000000"
                          "000000000000000000000001e+68")),
                      COIN / 100000000);
    BOOST_CHECK_EQUAL(
        AmountFromValue(ValueFromString("10000000000000000000000000000000000000"
                                        "000000000000000000000000000e-64")),
        COIN);
    BOOST_CHECK_EQUAL(
        AmountFromValue(ValueFromString(
            "0."
            "000000000000000000000000000000000000000000000000000000000000000100"
            "000000000000000000000000000000000000000000000000000e64")),
        COIN);

    // should fail
    BOOST_CHECK_THROW(AmountFromValue(ValueFromString("1e-9")), JSONRPCError);
    // should fail
    BOOST_CHECK_THROW(AmountFromValue(ValueFromString("0.000000019")), JSONRPCError);
    // should pass, cut trailing 0
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.00000001000000")),
                      FIXOSHI);
    // should fail
    BOOST_CHECK_THROW(AmountFromValue(ValueFromString("19e-9")), JSONRPCError);
    // should pass, leading 0 is present
    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.19e-6")),
                      19 * FIXOSHI);

    // overflow error
    BOOST_CHECK_THROW(AmountFromValue(ValueFromString("92233720368.54775808")), JSONRPCError);
    // overflow error
    BOOST_CHECK_THROW(AmountFromValue(ValueFromString("1e+11")), JSONRPCError);
    // overflow error signless
    BOOST_CHECK_THROW(AmountFromValue(ValueFromString("1e11")), JSONRPCError);
    // overflow error
    BOOST_CHECK_THROW(AmountFromValue(ValueFromString("93e+9")), JSONRPCError);
}

BOOST_AUTO_TEST_CASE(rpc_ban) {
    BOOST_CHECK_NO_THROW(CallRPC(std::string("clearbanned")));

    UniValue r;
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("setban 127.0.0.0 add")));
    // portnumber for setban not allowed
    BOOST_CHECK_THROW(r = CallRPC(std::string("setban 127.0.0.0:8334")), std::runtime_error);
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("listbanned")));
    {
        UniValue::Array& ar = r.get_array();
        UniValue::Object& o1 = ar.at(0).get_obj();
        UniValue& adr = o1.at("address");
        BOOST_CHECK_EQUAL(adr.get_str(), "127.0.0.0/32");
    }
    BOOST_CHECK_NO_THROW(CallRPC(std::string("setban 127.0.0.0 remove")));
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("listbanned")));
    {
        UniValue::Array& ar = r.get_array();
        BOOST_CHECK_EQUAL(ar.size(), 0UL);
    }

    // Set ban way in the future: 2283-12-18 19:33:20
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("setban 127.0.0.0/24 add 9907731200 true")));
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("listbanned")));
    {
        UniValue::Array& ar = r.get_array();
        UniValue::Object& o1 = ar.at(0).get_obj();
        UniValue& adr = o1.at("address");
        UniValue& banned_until = o1.at("banned_until");
        BOOST_CHECK_EQUAL(adr.get_str(), "127.0.0.0/24");
        // absolute time check
        BOOST_CHECK_EQUAL(banned_until.get_int64(), 9907731200);
    }

    BOOST_CHECK_NO_THROW(CallRPC(std::string("clearbanned")));

    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("setban 127.0.0.0/24 add 200")));
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("listbanned")));
    {
        UniValue::Array& ar = r.get_array();
        UniValue::Object& o1 = ar.at(0).get_obj();
        UniValue& adr = o1.at("address");
        UniValue& banned_until = o1.at("banned_until");
        BOOST_CHECK_EQUAL(adr.get_str(), "127.0.0.0/24");
        int64_t now = GetTime();
        BOOST_CHECK(banned_until.get_int64() > now);
        BOOST_CHECK(banned_until.get_int64() - now <= 200);
    }

    // must throw an exception because 127.0.0.1 is in already banned subnet
    // range
    BOOST_CHECK_THROW(r = CallRPC(std::string("setban 127.0.0.1 add")), std::runtime_error);

    BOOST_CHECK_NO_THROW(CallRPC(std::string("setban 127.0.0.0/24 remove")));
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("listbanned")));
    {
        UniValue::Array& ar = r.get_array();
        BOOST_CHECK_EQUAL(ar.size(), 0UL);
    }

    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("setban 127.0.0.0/255.255.0.0 add")));
    BOOST_CHECK_THROW(r = CallRPC(std::string("setban 127.0.1.1 add")), std::runtime_error);

    BOOST_CHECK_NO_THROW(CallRPC(std::string("clearbanned")));
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("listbanned")));
    {
        UniValue::Array& ar = r.get_array();
        BOOST_CHECK_EQUAL(ar.size(), 0UL);
    }

    // invalid IP
    BOOST_CHECK_THROW(r = CallRPC(std::string("setban test add")), std::runtime_error);

    // IPv6 tests
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("setban FE80:0000:0000:0000:0202:B3FF:FE1E:8329 add")));
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("listbanned")));
    {
        UniValue::Array& ar = r.get_array();
        UniValue::Object& o1 = ar.at(0).get_obj();
        UniValue& adr = o1.at("address");
        BOOST_CHECK_EQUAL(adr.get_str(), "fe80::202:b3ff:fe1e:8329/128");
    }

    BOOST_CHECK_NO_THROW(CallRPC(std::string("clearbanned")));
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("setban 2001:db8::/ffff:fffc:0:0:0:0:0:0 add")));
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("listbanned")));
    {
        UniValue::Array& ar = r.get_array();
        UniValue::Object& o1 = ar.at(0).get_obj();
        UniValue& adr = o1.at("address");
        BOOST_CHECK_EQUAL(adr.get_str(), "2001:db8::/30");
    }

    BOOST_CHECK_NO_THROW(CallRPC(std::string("clearbanned")));
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("setban 2001:4d48:ac57:400:cacf:e9ff:fe1d:9c63/128 add")));
    BOOST_CHECK_NO_THROW(r = CallRPC(std::string("listbanned")));
    {
        UniValue::Array& ar = r.get_array();
        UniValue::Object& o1 = ar.at(0).get_obj();
        UniValue& adr = o1.at("address");
        BOOST_CHECK_EQUAL(adr.get_str(), "2001:4d48:ac57:400:cacf:e9ff:fe1d:9c63/128");
    }
}

BOOST_AUTO_TEST_CASE(rpc_convert_values_generatetoaddress) {
    UniValue result;

    BOOST_CHECK_NO_THROW(result = RPCConvertValues(
                             "generatetoaddress",
                             {"101", "mkESjLZW66TmHhiFX8MCaBjrhZ543PPh9a"}));
    BOOST_CHECK_EQUAL(result[0].get_int(), 101);
    BOOST_CHECK_EQUAL(result[1].get_str(),
                      "mkESjLZW66TmHhiFX8MCaBjrhZ543PPh9a");

    BOOST_CHECK_NO_THROW(result = RPCConvertValues(
                             "generatetoaddress",
                             {"101", "mhMbmE2tE9xzJYCV9aNC8jKWN31vtGrguU"}));
    BOOST_CHECK_EQUAL(result[0].get_int(), 101);
    BOOST_CHECK_EQUAL(result[1].get_str(),
                      "mhMbmE2tE9xzJYCV9aNC8jKWN31vtGrguU");

    BOOST_CHECK_NO_THROW(result = RPCConvertValues(
                             "generatetoaddress",
                             {"1", "mkESjLZW66TmHhiFX8MCaBjrhZ543PPh9a", "9"}));
    BOOST_CHECK_EQUAL(result[0].get_int(), 1);
    BOOST_CHECK_EQUAL(result[1].get_str(),
                      "mkESjLZW66TmHhiFX8MCaBjrhZ543PPh9a");
    BOOST_CHECK_EQUAL(result[2].get_int(), 9);

    BOOST_CHECK_NO_THROW(result = RPCConvertValues(
                             "generatetoaddress",
                             {"1", "mhMbmE2tE9xzJYCV9aNC8jKWN31vtGrguU", "9"}));
    BOOST_CHECK_EQUAL(result[0].get_int(), 1);
    BOOST_CHECK_EQUAL(result[1].get_str(),
                      "mhMbmE2tE9xzJYCV9aNC8jKWN31vtGrguU");
    BOOST_CHECK_EQUAL(result[2].get_int(), 9);
}

BOOST_AUTO_TEST_CASE(rpc_getblockstats_calculate_percentiles_by_size)
{
    int64_t total_size = 200;
    std::vector<std::pair<Amount, int64_t>> feerates;
    Amount result[NUM_GETBLOCKSTATS_PERCENTILES] = { Amount::zero() };

    for (int64_t i = 0; i < 100; i++) {
        feerates.emplace_back(std::make_pair(Amount(1 * FIXOSHI) ,1));
    }

    for (int64_t i = 0; i < 100; i++) {
        feerates.emplace_back(std::make_pair(Amount(2 * FIXOSHI) ,1));
    }

    CalculatePercentilesBySize(result, feerates, total_size);
    BOOST_CHECK_EQUAL(result[0], Amount(1 * FIXOSHI));
    BOOST_CHECK_EQUAL(result[1], Amount(1 * FIXOSHI));
    BOOST_CHECK_EQUAL(result[2], Amount(1 * FIXOSHI));
    BOOST_CHECK_EQUAL(result[3], Amount(2 * FIXOSHI));
    BOOST_CHECK_EQUAL(result[4], Amount(2 * FIXOSHI));

    // Test with more pairs, and two pairs overlapping 2 percentiles.
    total_size = 100;
    Amount result2[NUM_GETBLOCKSTATS_PERCENTILES] = { Amount::zero() };
    feerates.clear();

    feerates.emplace_back(std::make_pair(Amount(1 * FIXOSHI), 9));
    feerates.emplace_back(std::make_pair(Amount(2 * FIXOSHI), 16)); //10th + 25th percentile
    feerates.emplace_back(std::make_pair(Amount(4 * FIXOSHI), 50)); //50th + 75th percentile
    feerates.emplace_back(std::make_pair(Amount(5 * FIXOSHI), 10));
    feerates.emplace_back(std::make_pair(Amount(9 * FIXOSHI), 15));  // 90th percentile

    CalculatePercentilesBySize(result2, feerates, total_size);

    BOOST_CHECK_EQUAL(result2[0], Amount(2 * FIXOSHI));
    BOOST_CHECK_EQUAL(result2[1], Amount(2 * FIXOSHI));
    BOOST_CHECK_EQUAL(result2[2], Amount(4 * FIXOSHI));
    BOOST_CHECK_EQUAL(result2[3], Amount(4 * FIXOSHI));
    BOOST_CHECK_EQUAL(result2[4], Amount(9 * FIXOSHI));

    // Same test as above, but one of the percentile-overlapping pairs is split in 2.
    total_size = 100;
    Amount result3[NUM_GETBLOCKSTATS_PERCENTILES] = { Amount::zero() };
    feerates.clear();

    feerates.emplace_back(std::make_pair(Amount(1 * FIXOSHI), 9));
    feerates.emplace_back(std::make_pair(Amount(2 * FIXOSHI), 11)); // 10th percentile
    feerates.emplace_back(std::make_pair(Amount(2 * FIXOSHI), 5)); // 25th percentile
    feerates.emplace_back(std::make_pair(Amount(4 * FIXOSHI), 50)); //50th + 75th percentile
    feerates.emplace_back(std::make_pair(Amount(5 * FIXOSHI), 10));
    feerates.emplace_back(std::make_pair(Amount(9 * FIXOSHI), 15)); // 90th percentile

    CalculatePercentilesBySize(result3, feerates, total_size);

    BOOST_CHECK_EQUAL(result3[0], Amount(2 * FIXOSHI));
    BOOST_CHECK_EQUAL(result3[1], Amount(2 * FIXOSHI));
    BOOST_CHECK_EQUAL(result3[2], Amount(4 * FIXOSHI));
    BOOST_CHECK_EQUAL(result3[3], Amount(4 * FIXOSHI));
    BOOST_CHECK_EQUAL(result3[4], Amount(9 * FIXOSHI));

    // Test with one transaction spanning all percentiles.
    total_size = 104;
    Amount result4[NUM_GETBLOCKSTATS_PERCENTILES] = { Amount::zero() };
    feerates.clear();

    feerates.emplace_back(std::make_pair(Amount(1 * FIXOSHI), 100));
    feerates.emplace_back(std::make_pair(Amount(2 * FIXOSHI), 1));
    feerates.emplace_back(std::make_pair(Amount(3 * FIXOSHI), 1));
    feerates.emplace_back(std::make_pair(Amount(3 * FIXOSHI), 1));
    feerates.emplace_back(std::make_pair(Amount(999999 * FIXOSHI), 1));

    CalculatePercentilesBySize(result4, feerates, total_size);

    for (int64_t i = 0; i < NUM_GETBLOCKSTATS_PERCENTILES; i++) {
        BOOST_CHECK_EQUAL(result4[i], Amount(1 * FIXOSHI));
    }
}

BOOST_AUTO_TEST_SUITE_END()
