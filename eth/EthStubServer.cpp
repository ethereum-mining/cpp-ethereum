/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file EthStubServer.cpp
 * @authors:
 *   Gav Wood <i@gavwood.com>
 * @date 2014
 */

//#if ETH_JSONRPC
#include "EthStubServer.h"
#include <libevmface/Instruction.h>
#include <liblll/Compiler.h>
#include <libethereum/Client.h>
#include <libwebthree/WebThree.h>
#include <libdevcore/CommonJS.h>
using namespace std;
using namespace dev;
using namespace dev::eth;

EthStubServer::EthStubServer(jsonrpc::AbstractServerConnector* _conn, WebThreeDirect& _web3):
	AbstractEthStubServer(_conn),
	m_web3(_web3)
{
}

dev::eth::Interface* EthStubServer::client() const
{
    return &(*m_web3.ethereum());
}

std::string EthStubServer::balanceAt(const string &a, const int& block)
{
    return toJS(client()->balanceAt(jsToAddress(a), block));
}

//TODO BlockDetails?
Json::Value EthStubServer::block(const string &numberOrHash)
{
    auto n = jsToU256(numberOrHash);
    auto h = n < client()->number() ? client()->hashFromNumber((unsigned)n) : ::jsToFixed<32>(numberOrHash);

    Json::Value res;
    BlockInfo bi = client()->blockInfo(h);
    res["hash"] = boost::lexical_cast<string>(bi.hash);

    res["parentHash"] = boost::lexical_cast<string>(bi.parentHash);
    res["sha3Uncles"] = boost::lexical_cast<string>(bi.sha3Uncles);
    res["miner"] = boost::lexical_cast<string>(bi.coinbaseAddress);
    res["stateRoot"] = boost::lexical_cast<string>(bi.stateRoot);
    res["transactionsRoot"] = boost::lexical_cast<string>(bi.transactionsRoot);
    res["difficulty"] = boost::lexical_cast<string>(bi.difficulty);
    res["number"] = boost::lexical_cast<string>(bi.number);
    res["minGasPrice"] = boost::lexical_cast<string>(bi.minGasPrice);
    res["gasLimit"] = boost::lexical_cast<string>(bi.gasLimit);
    res["timestamp"] = boost::lexical_cast<string>(bi.timestamp);
    res["extraData"] = jsFromBinary(bi.extraData);
    res["nonce"] = boost::lexical_cast<string>(bi.nonce);
    return res;
}

static TransactionJS toTransaction(const Json::Value &json)
{
    TransactionJS ret;
    if (!json.isObject() || json.empty()){
        return ret;
    }

    if (!json["from"].empty())
        ret.from = jsToSecret(json["from"].asString());
    if (!json["to"].empty())
        ret.to = jsToAddress(json["to"].asString());
    if (!json["value"].empty())
        ret.value = jsToU256(json["value"].asString());
    if (!json["gas"].empty())
        ret.gas = jsToU256(json["gas"].asString());
    if (!json["gasPrice"].empty())
        ret.gasPrice = jsToU256(json["gasPrice"].asString());

    if (!json["data"].empty() || json["code"].empty() || json["dataclose"].empty())
    {
        if (json["data"].isString())
            ret.data = jsToBytes(json["data"].asString());
        else if (json["code"].isString())
            ret.data = jsToBytes(json["code"].asString());
        else if (json["data"].isArray())
            for (auto i: json["data"])
                dev::operator +=(ret.data, asBytes(jsPadded(i.asString(), 32)));
        else if (json["code"].isArray())
            for (auto i: json["code"])
                dev::operator +=(ret.data, asBytes(jsPadded(i.asString(), 32)));
        else if (json["dataclose"].isArray())
            for (auto i: json["dataclose"])
                dev::operator +=(ret.data, jsToBytes(i.asString()));
    }

    return ret;
}

std::string EthStubServer::call(const Json::Value &json)
{
    std::string ret;
    if (!client())
        return ret;
    TransactionJS t = toTransaction(json);
    if (!t.to)
        return ret;
    if (!t.from && m_keys.size())
        t.from = m_keys[0].secret();
    if (!t.gasPrice)
        t.gasPrice = 10 * dev::eth::szabo;
    if (!t.gas)
        t.gas = client()->balanceAt(KeyPair(t.from).address()) / t.gasPrice;
    ret = toJS(client()->call(t.from, t.value, t.to, t.data, t.gas, t.gasPrice));
    return ret;
}

std::string EthStubServer::codeAt(const string &a, const int& block)
{
    return client() ? jsFromBinary(client()->codeAt(jsToAddress(a), block)) : "";
}

std::string EthStubServer::coinbase()
{
    return client() ? toJS(client()->address()) : "";
}

std::string EthStubServer::countAt(const string &a, const int& block)
{
    return client() ? toJS(client()->countAt(jsToAddress(a), block)) : "";
}

int EthStubServer::defaultBlock()
{
    return client() ? client()->getDefault() : 0;
}

std::string EthStubServer::fromAscii(const int& padding, const std::string& s)
{
    return jsFromBinary(s, padding);
}

double EthStubServer::fromFixed(const string &s)
{
    return jsFromFixed(s);
}

std::string EthStubServer::gasPrice()
{
    return toJS(10 * dev::eth::szabo);
}

//TODO
bool EthStubServer::isListening()
{
    return /*client() ? client()->haveNetwork() :*/ false;
}

bool EthStubServer::isMining()
{
    return client() ? client()->isMining() : false;
}

std::string EthStubServer::key()
{
    if (!m_keys.size())
        return std::string();
    return toJS(m_keys[0].sec());
}

Json::Value EthStubServer::keys()
{
    Json::Value ret;
    for (auto i: m_keys)
        ret.append(toJS(i.secret()));
    return ret;
}

std::string EthStubServer::lll(const string &s)
{
    return toJS(dev::eth::compileLLL(s));
}

std::string EthStubServer::messages(const string &json)
{

}

int EthStubServer::number()
{

}

int EthStubServer::peerCount()
{
    return m_web3.peerCount();
}

std::string EthStubServer::secretToAddress(const string &s)
{

}

std::string EthStubServer::setListening(const string &l)
{

}

std::string EthStubServer::setMining(const string &l)
{

}

std::string EthStubServer::sha3(const string &s)
{

}

std::string EthStubServer::stateAt(const string &a, const int& block, const string &p)
{

}

std::string EthStubServer::toAscii(const string &s)
{

}

std::string EthStubServer::toDecimal(const string &s)
{

}

std::string EthStubServer::toFixed(const string &s)
{

}

std::string EthStubServer::transact(const string &json)
{

}

std::string EthStubServer::transaction(const string &i, const string &numberOrHash)
{

}

std::string EthStubServer::uncle(const string &i, const string &numberOrHash)
{

}

std::string EthStubServer::watch(const string &json)
{

}

Json::Value EthStubServer::blockJson(const std::string& _hash)
{
	Json::Value res;
//    auto const& bc = client()->blockChain();
	
//	auto b = _hash.length() ? bc.block(h256(_hash)) : bc.block();
	
//	auto bi = BlockInfo(b);
//	res["number"] = boost::lexical_cast<string>(bi.number);
//	res["hash"] = boost::lexical_cast<string>(bi.hash);
//	res["parentHash"] = boost::lexical_cast<string>(bi.parentHash);
//	res["sha3Uncles"] = boost::lexical_cast<string>(bi.sha3Uncles);
//	res["coinbaseAddress"] = boost::lexical_cast<string>(bi.coinbaseAddress);
//	res["stateRoot"] = boost::lexical_cast<string>(bi.stateRoot);
//	res["transactionsRoot"] = boost::lexical_cast<string>(bi.transactionsRoot);
//	res["minGasPrice"] = boost::lexical_cast<string>(bi.minGasPrice);
//	res["gasLimit"] = boost::lexical_cast<string>(bi.gasLimit);
//	res["gasUsed"] = boost::lexical_cast<string>(bi.gasUsed);
//	res["difficulty"] = boost::lexical_cast<string>(bi.difficulty);
//	res["timestamp"] = boost::lexical_cast<string>(bi.timestamp);
//	res["nonce"] = boost::lexical_cast<string>(bi.nonce);

	return res;
}

Json::Value EthStubServer::jsontypeToValue(int _jsontype)
{
	switch (_jsontype)
	{
		case jsonrpc::JSON_STRING: return ""; //Json::stringValue segfault, fuck knows why
		case jsonrpc::JSON_BOOLEAN: return Json::booleanValue;
		case jsonrpc::JSON_INTEGER: return Json::intValue;
		case jsonrpc::JSON_REAL: return Json::realValue;
		case jsonrpc::JSON_OBJECT: return Json::objectValue;
		case jsonrpc::JSON_ARRAY: return Json::arrayValue;
		default: return Json::nullValue;
	}
}

//#endif
