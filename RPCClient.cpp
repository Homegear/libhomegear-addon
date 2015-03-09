/* Copyright 2013-2015 Sathya Laufer
 *
 * Homegear is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Homegear is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Homegear.  If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */

#include "RPCClient.h"
#include "GD.h"

namespace HgAddonLib
{

RPCClient::RPCClient()
{
	try
	{
		signal(SIGPIPE, SIG_IGN);
	}
	catch(const std::exception& ex)
    {
    	GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

RPCClient::~RPCClient()
{
	try
	{

	}
	catch(const std::exception& ex)
    {
    	GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RPCClient::setPort(int32_t port)
{
	_port = port;
}

PVariable RPCClient::invoke(std::string methodName, PRPCList parameters)
{
	try
	{
		if(methodName.empty()) return Variable::createError(-32601, "Method name is empty");
		GD::out.printInfo("Info: Calling XML RPC method \"" + methodName + "\".");
		if(GD::debugLevel >= 5 && parameters)
		{
			GD::out.printDebug("Parameters:");
			for(RPCList::iterator i = parameters->begin(); i != parameters->end(); ++i)
			{
				(*i)->print();
			}
		}
		bool retry = false;
		std::vector<char> requestData;
		std::vector<char> responseData;
		_rpcEncoder.encodeRequest(methodName, parameters, requestData);
		for(uint32_t i = 0; i < 3; ++i)
		{
			retry = false;
			if(i == 0) sendRequest(requestData, responseData, true, retry);
			else sendRequest(requestData, responseData, false, retry);
			if(!retry) break;
		}
		if(retry) return Variable::createError(-32300, "Request timed out.");
		if(responseData.empty()) return Variable::createError(-32700, "No response data.");
		PVariable returnValue;
		returnValue = _rpcDecoder.decodeResponse(responseData);
		if(returnValue->errorStruct) GD::out.printError("Error in RPC response: faultCode: " + std::to_string(returnValue->structValue->at("faultCode")->integerValue) + " faultString: " + returnValue->structValue->at("faultString")->stringValue);
		else
		{
			if(GD::debugLevel >= 5)
			{
				GD::out.printDebug("Response was:");
				returnValue->print();
			}
		}
		return returnValue;
	}
    catch(const std::exception& ex)
    {
    	GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return Variable::createError(-32700, "No response data.");
}

void RPCClient::sendRequest(std::vector<char>& data, std::vector<char>& responseData, bool insertHeader, bool& retry)
{
	_sendMutex.lock();
	try
	{
		try
		{
			if(!_socket.connected())
			{
				_socket.setHostname("127.0.0.1");
				_socket.setPort(std::to_string(_port));
				_socket.open();
			}
		}
		catch(const SocketOperationException& ex)
		{
			GD::out.printError(ex.what());
			_sendMutex.unlock();
			return;
		}

		if(GD::debugLevel >= 5) GD::out.printDebug("Sending packet: " + GD::hf.getHexString(data));

		try
		{
			_socket.proofwrite(data);
		}
		catch(SocketDataLimitException& ex)
		{
			GD::out.printWarning("Warning: " + ex.what());
			_socket.close();
			_sendMutex.unlock();
			return;
		}
		catch(const SocketOperationException& ex)
		{
			GD::out.printError("Error: Could not send data to Homegear: " + ex.what() + ".");
			retry = true;
			_socket.close();
			_sendMutex.unlock();
			return;
		}

		responseData.clear();
		ssize_t receivedBytes;

		int32_t bufferMax = 2048;
		char buffer[bufferMax + 1];
		uint32_t packetLength = 0;
		uint32_t dataSize = 0;

		while(true) //This is equal to while(true) for binary packets
		{
			try
			{
				receivedBytes = _socket.proofread(buffer, bufferMax);

				//Some clients send only one byte in the first packet
				if(packetLength == 0 && receivedBytes == 1) receivedBytes += _socket.proofread(&buffer[1], bufferMax - 1);
			}
			catch(const SocketTimeOutException& ex)
			{
				GD::out.printInfo("Info: Reading from Homegear timed out.");
				retry = true;
				_sendMutex.unlock();
				return;
			}
			catch(const SocketClosedException& ex)
			{
				GD::out.printWarning("Warning: " + ex.what());
				retry = true;
				_sendMutex.unlock();
				return;
			}
			catch(const SocketOperationException& ex)
			{
				GD::out.printError(ex.what());
				retry = true;
				_sendMutex.unlock();
				return;
			}
			//We are using string functions to process the buffer. So just to make sure,
			//they don't do something in the memory after buffer, we add '\0'
			buffer[receivedBytes] = '\0';

			if(dataSize == 0)
			{
				if(!(buffer[3] & 1) && buffer[3] != 0xFF)
				{
					responseData.insert(responseData.end(), buffer, buffer + receivedBytes);
					GD::out.printError("Error: RPC client received binary request as response from Homegear. Packet was: " + GD::hf.getHexString(responseData));
					_sendMutex.unlock();
					return;
				}
				if(receivedBytes < 8)
				{
					GD::out.printError("Error: RPC client received binary packet smaller than 8 bytes from Homegear.");
					_sendMutex.unlock();
					return;
				}
				GD::hf.memcpyBigEndian((char*)&dataSize, buffer + 4, 4);
				GD::out.printDebug("RPC client receiving binary rpc packet with size: " + std::to_string(receivedBytes) + ". Payload size is: " + std::to_string(dataSize));
				if(dataSize == 0)
				{
					GD::out.printError("Error: RPC client received binary packet without data from Homegear.");
					_sendMutex.unlock();
					return;
				}
				if(dataSize > 10485760)
				{
					GD::out.printError("Error: RPC client received packet with data larger than 10 MiB.");
					_sendMutex.unlock();
					return;
				}
				packetLength = receivedBytes - 8;
				responseData.insert(responseData.end(), buffer, buffer + receivedBytes);
			}
			else
			{
				if(packetLength + receivedBytes > dataSize)
				{
					GD::out.printError("Error: RPC client received response packet larger than the expected data size.");
					_sendMutex.unlock();
					return;
				}
				responseData.insert(responseData.end(), buffer, buffer + receivedBytes);
				packetLength += receivedBytes;
			}
			if(packetLength == dataSize)
			{
				responseData.push_back('\0');
				break;
			}
		}
		if(GD::debugLevel >= 5) GD::out.printDebug("Debug: Received packet from Homegear: " + GD::hf.getHexString(responseData));
		_sendMutex.unlock();
		return;
    }
    catch(const std::exception& ex)
    {
    	GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    _socket.shutdown();
    _sendMutex.unlock();
}
}
