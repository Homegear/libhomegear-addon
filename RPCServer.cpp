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

#include "RPCServer.h"
#include "GD.h"
#include "RPCMethods.h"

namespace HgAddonLib
{
RPCServer::RPCServer()
{
	_out.setPrefix("RPC Server: ");
}

RPCServer::~RPCServer()
{
	stop();
}

void RPCServer::start(Base* base, uint64_t myPeerId)
{
	try
	{
		if(myPeerId == 0)
		{
			GD::out.printCritical("Critical: Could not start. myPeerId is \"0\".");
			return;
		}
		_myPeerId = myPeerId;
		if(_myPeerId != 0) _subscribedPeers.insert(_myPeerId);
		stop();
		_stopServer = false;
		registerMethods(base);
		getSocketDescriptor();
		_mainThread = std::thread(&RPCServer::mainThread, this);
		std::string id = GD::rpcServer.getId();
		if(id.empty())
		{
			GD::out.printCritical("Critical: Could not get server id. Aborting start.");
			stop();
			return;
		}
		sendInit();
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RPCServer::stop()
{
	try
	{
		_stopServer = true;
		if(_mainThread.joinable()) _mainThread.join();
		_rpcMethods.clear();
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RPCServer::addPeers(std::vector<uint64_t>& peerIds)
{
	try
	{
		//Mutex does not need to be locked, as no iterators are invalidated
		for(std::vector<uint64_t>::iterator i = peerIds.begin(); i != peerIds.end(); ++i)
		{
			_subscribedPeers.insert(*i);
		}
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RPCServer::removePeers(std::vector<uint64_t>& peerIds)
{
	try
	{
		_subscribedPeersMutex.lock();
		for(std::vector<uint64_t>::iterator i = peerIds.begin(); i != peerIds.end(); ++i)
		{
			_subscribedPeers.erase(*i);
		}
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    _subscribedPeersMutex.unlock();
}

void RPCServer::registerMethods(Base* base)
{
	try
	{
		_rpcMethods.clear();
		_rpcMethods.insert(std::pair<std::string, std::unique_ptr<RPCMethod>>("system.listMethods", std::unique_ptr<RPCMethod>(new RPCSystemListMethods())));
		_rpcMethods.insert(std::pair<std::string, std::unique_ptr<RPCMethod>>("system.methodHelp", std::unique_ptr<RPCMethod>(new RPCSystemMethodHelp())));
		_rpcMethods.insert(std::pair<std::string, std::unique_ptr<RPCMethod>>("system.methodSignature", std::unique_ptr<RPCMethod>(new RPCSystemMethodSignature())));
		_rpcMethods.insert(std::pair<std::string, std::unique_ptr<RPCMethod>>("system.multicall", std::unique_ptr<RPCMethod>(new RPCSystemMulticall())));
		_rpcMethods.insert(std::pair<std::string, std::unique_ptr<RPCMethod>>("deleteDevices", std::unique_ptr<RPCMethod>(new RPCDeleteDevices(base))));
		_rpcMethods.insert(std::pair<std::string, std::unique_ptr<RPCMethod>>("error", std::unique_ptr<RPCMethod>(new RPCError(base))));
		_rpcMethods.insert(std::pair<std::string, std::unique_ptr<RPCMethod>>("event", std::unique_ptr<RPCMethod>(new RPCEvent(base))));
		_rpcMethods.insert(std::pair<std::string, std::unique_ptr<RPCMethod>>("newDevices", std::unique_ptr<RPCMethod>(new RPCNewDevices(base))));
		_rpcMethods.insert(std::pair<std::string, std::unique_ptr<RPCMethod>>("updateDevice", std::unique_ptr<RPCMethod>(new RPCUpdateDevice(base))));
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RPCServer::mainThread()
{
	try
	{
		std::string address;
		int32_t clientSocketDescriptor = -1;
		SocketOperations socket;
		while(!_stopServer)
		{
			try
			{
				sendInit();
				if(_serverSocketDescriptor == -1)
				{
					if(_stopServer) break;
					std::this_thread::sleep_for(std::chrono::milliseconds(5000));
					getSocketDescriptor();
					continue;
				}
				for(int32_t i = 0; i < 6; i++)
				{
					clientSocketDescriptor = getClientSocketDescriptor();
					if(clientSocketDescriptor != -1 || _stopServer) break;
				}
				if(clientSocketDescriptor == -1) continue;

				socket = SocketOperations(clientSocketDescriptor);
				readClient(socket);
			}
			catch(const std::exception& ex)
			{
				_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
			}
			catch(Exception& ex)
			{
				_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
			}
			catch(...)
			{
				_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
			}
		}
		std::string id = GD::rpcServer.getId();
		if(!id.empty()) GD::rpcClient.invoke("init", RPCCLIENTPARAMETERS(id, std::string("")));
		shutdown(_serverSocketDescriptor, 0);
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RPCServer::sendRPCResponseToClient(SocketOperations& socket, std::vector<char>& data)
{
	try
	{
		if(data.empty()) return;
		bool error = false;
		try
		{
			//Sleep a tiny little bit.
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
			socket.proofwrite(data);
		}
		catch(SocketDataLimitException& ex)
		{
			_out.printWarning("Warning: " + ex.what());
		}
		catch(const SocketOperationException& ex)
		{
			_out.printError("Error: " + ex.what());
			error = true;
		}
		if(error) socket.close();
	}
    catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RPCServer::analyzeRPC(SocketOperations& socket, std::vector<char>& packet)
{
	try
	{
		std::string methodName;
		std::shared_ptr<std::vector<std::shared_ptr<Variable>>> parameters;
		parameters = _rpcDecoder.decodeRequest(packet, methodName);
		if(!parameters)
		{
			_out.printWarning("Warning: Could not decode RPC packet.");
			return;
		}
		if(!parameters->empty() && parameters->at(0)->errorStruct)
		{
			sendRPCResponseToClient(socket, parameters->at(0));
			return;
		}
		callMethod(socket, methodName, parameters);
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RPCServer::sendRPCResponseToClient(SocketOperations& socket, std::shared_ptr<Variable> variable)
{
	try
	{
		std::vector<char> data;
		_rpcEncoder.encodeResponse(variable, data);
		if(GD::debugLevel >= 5)
		{
			_out.printDebug("Response binary:");
			_out.printBinary(data);
		}
		sendRPCResponseToClient(socket, data);
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

std::shared_ptr<Variable> RPCServer::callMethod(std::string& methodName, std::shared_ptr<Variable>& parameters)
{
	try
	{
		if(!parameters) parameters = std::shared_ptr<Variable>(new Variable(VariableType::rpcArray));
		if(_rpcMethods.find(methodName) == _rpcMethods.end())
		{
			_out.printError("Warning: RPC method not found: " + methodName);
			return Variable::createError(-32601, ": Requested method not found.");
		}
		if(GD::debugLevel >= 4)
		{
			_out.printInfo("Info: RPC Method called: " + methodName + " Parameters:");
			for(std::vector<std::shared_ptr<Variable>>::iterator i = parameters->arrayValue->begin(); i != parameters->arrayValue->end(); ++i)
			{
				(*i)->print();
			}
		}
		std::shared_ptr<Variable> ret = _rpcMethods.at(methodName)->invoke(parameters->arrayValue);
		if(GD::debugLevel >= 5)
		{
			_out.printDebug("Response: ");
			ret->print();
		}
		return ret;
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return Variable::createError(-32500, ": Unknown application error.");
}

void RPCServer::callMethod(SocketOperations& socket, std::string methodName, std::shared_ptr<std::vector<std::shared_ptr<Variable>>> parameters)
{
	try
	{
		if(_rpcMethods.find(methodName) == _rpcMethods.end())
		{
			sendRPCResponseToClient(socket, Variable::createError(-32601, ": Requested method not found."));
			return;
		}
		if(GD::debugLevel >= 4)
		{
			_out.printInfo("Info: Client is calling RPC method: " + methodName + " Parameters:");
			for(std::vector<std::shared_ptr<Variable>>::iterator i = parameters->begin(); i != parameters->end(); ++i)
			{
				(*i)->print();
			}
		}
		std::shared_ptr<Variable> ret = _rpcMethods.at(methodName)->invoke(parameters);
		if(GD::debugLevel >= 5)
		{
			_out.printDebug("Response: ");
			ret->print();
		}
		sendRPCResponseToClient(socket, ret);
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RPCServer::sendInit()
{
	try
	{
		_lastKeepAlive = GD::hf.getTimeSeconds();
		std::string id = GD::rpcServer.getId();
		if(id.empty()) return;
		if(GD::hf.getTimeSeconds() - _lastInit < 30) return;
		_lastInit = GD::hf.getTimeSeconds();
		if(GD::rpcClient.invoke("init", RPCCLIENTPARAMETERS(id, id + "-AddonLib", 15))->errorStruct) return;
		std::shared_ptr<RPCArray> subscribedPeers(new RPCArray());
		_subscribedPeersMutex.lock();
		for(std::set<uint64_t>::const_iterator i = _subscribedPeers.begin(); i != _subscribedPeers.end(); ++i)
		{
			subscribedPeers->push_back(PVariable(new Variable((uint32_t)*i)));
		}
		_subscribedPeersMutex.unlock();
		GD::rpcClient.invoke("setClientType", RPCCLIENTPARAMETERS(1));
		GD::rpcClient.invoke("subscribePeers", RPCCLIENTPARAMETERS(id, subscribedPeers));
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RPCServer::keepAlive()
{
	try
	{
		if(GD::hf.getTimeSeconds() - _lastKeepAlive >= 30)
		{
			_lastKeepAlive = GD::hf.getTimeSeconds();
			bool result = GD::rpcClient.invoke("clientServerInitialized", RPCCLIENTPARAMETERS(getId() + "-AddonLib"))->booleanValue;
			if(!result) sendInit();
		}
	}
	catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void RPCServer::readClient(SocketOperations& socket)
{
	try
	{
		int32_t bufferMax = 1024;
		char buffer[bufferMax + 1];
		//Make sure the buffer is null terminated.
		buffer[bufferMax] = '\0';
		std::vector<char> packet;
		uint32_t packetLength = 0;
		int32_t bytesRead;
		uint32_t dataSize = 0;

		while(!_stopServer)
		{
			try
			{
				bytesRead = socket.proofread(buffer, bufferMax);
				buffer[bufferMax] = 0; //Even though it shouldn't matter, make sure there is a null termination.
				//Some clients send only one byte in the first packet
				if(packetLength == 0 && bytesRead == 1) bytesRead += socket.proofread(&buffer[1], bufferMax - 1);
			}
			catch(const SocketTimeOutException& ex)
			{
				keepAlive();
				continue;
			}
			catch(const SocketClosedException& ex)
			{
				_out.printInfo("Info: " + ex.what());
				break;
			}
			catch(const SocketOperationException& ex)
			{
				_out.printError(ex.what());
				break;
			}

			if(GD::debugLevel >= 5)
			{
				std::vector<uint8_t> rawPacket(buffer, buffer + bytesRead);
				_out.printDebug("Debug: Packet received: " + HelperFunctions::getHexString(rawPacket));
			}
			if(packetLength == 0 && !strncmp(&buffer[0], "Bin", 3))
			{
				if(bytesRead < 8) continue;
				uint32_t headerSize = 0;
				if(buffer[3] & 0x40)
				{
					GD::hf.memcpyBigEndian((char*)&headerSize, buffer + 4, 4);
					if(bytesRead < (signed)headerSize + 12)
					{
						_out.printError("Error: Binary rpc packet has invalid header size.");
						continue;
					}
					GD::hf.memcpyBigEndian((char*)&dataSize, buffer + 8 + headerSize, 4);
					dataSize += headerSize + 4;
				}
				else GD::hf.memcpyBigEndian((char*)&dataSize, buffer + 4, 4);
				_out.printDebug("Receiving binary rpc packet with size: " + std::to_string(dataSize), 6);
				if(dataSize == 0) continue;
				if(headerSize > 1024)
				{
					_out.printError("Error: Binary rpc packet with header larger than 1 KiB received.");
					continue;
				}
				if(dataSize > 10485760)
				{
					_out.printError("Error: Packet with data larger than 10 MiB received.");
					continue;
				}
				packet.clear();
				packet.reserve(dataSize + 9);
				packet.insert(packet.end(), buffer, buffer + bytesRead);
				std::shared_ptr<RPCHeader> header = _rpcDecoder.decodeHeader(packet);

				if(dataSize > (unsigned)bytesRead - 8) packetLength = bytesRead - 8;
				else
				{
					packetLength = 0;
					analyzeRPC(socket, packet);
				}
			}
			else if(packetLength > 0)
			{
				if(packetLength + bytesRead > dataSize)
				{
					_out.printError("Error: Packet length is wrong.");
					packetLength = 0;
					continue;
				}
				packet.insert(packet.end(), buffer, buffer + bytesRead);
				packetLength += bytesRead;
				if(packetLength == dataSize)
				{
					packet.push_back('\0');
					analyzeRPC(socket, packet);
					packetLength = 0;
				}
			}
			else
			{
				_out.printError("Error: Uninterpretable packet received. Closing connection. Packet was: " + std::string(buffer, bytesRead));
				break;
			}
		}
	}
    catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    //This point is only reached, when stopServer is true, the socket is closed or an error occured
	socket.close();
}

int32_t RPCServer::getClientSocketDescriptor()
{
	int32_t socketDescriptor = -1;
	try
	{
		timeval timeout;
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
		fd_set readFileDescriptor;
		FD_ZERO(&readFileDescriptor);
		int32_t nfds = _serverSocketDescriptor + 1;
		if(nfds <= 0)
		{
			GD::out.printError("Error: Server file descriptor is invalid.");
			return socketDescriptor;
		}
		FD_SET(_serverSocketDescriptor, &readFileDescriptor);
		if(!select(nfds, &readFileDescriptor, NULL, NULL, &timeout))
		{
			return socketDescriptor;
		}

		struct sockaddr_storage clientInfo;
		socklen_t addressSize = sizeof(addressSize);
		socketDescriptor = accept(_serverSocketDescriptor, (struct sockaddr *) &clientInfo, &addressSize);
		if(socketDescriptor == -1) return socketDescriptor;

		getpeername(socketDescriptor, (struct sockaddr*)&clientInfo, &addressSize);

		char ipString[INET6_ADDRSTRLEN];
		if (clientInfo.ss_family == AF_INET) {
			struct sockaddr_in *s = (struct sockaddr_in *)&clientInfo;
			inet_ntop(AF_INET, &s->sin_addr, ipString, sizeof(ipString));
		} else { // AF_INET6
			struct sockaddr_in6 *s = (struct sockaddr_in6 *)&clientInfo;
			inet_ntop(AF_INET6, &s->sin6_addr, ipString, sizeof(ipString));
		}

		_out.printInfo("Info: Connection from " + std::string(&ipString[0]) + " accepted.");
	}
    catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return socketDescriptor;
}

void RPCServer::getSocketDescriptor()
{
	try
	{
		addrinfo hostInfo;
		addrinfo *serverInfo = nullptr;

		int32_t yes = 1;

		memset(&hostInfo, 0, sizeof(hostInfo));

		hostInfo.ai_family = AF_UNSPEC;
		hostInfo.ai_socktype = SOCK_STREAM;
		hostInfo.ai_flags = AI_PASSIVE;
		int32_t result;
		if((result = getaddrinfo("0.0.0.0", NULL, &hostInfo, &serverInfo)) != 0)
		{
			_out.printCritical("Error: Could not get address information: " + std::string(gai_strerror(result)));
			return;
		}

		bool bound = false;
		int32_t error = 0;
		for(struct addrinfo *info = serverInfo; info != 0; info = info->ai_next)
		{
			_serverSocketDescriptor = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
			if(_serverSocketDescriptor == -1) continue;
			if(!(fcntl(_serverSocketDescriptor, F_GETFL) & O_NONBLOCK))
			{
				if(fcntl(_serverSocketDescriptor, F_SETFL, fcntl(_serverSocketDescriptor, F_GETFL) | O_NONBLOCK) < 0)
				{
					_out.printError("Error: Could not set socket options.");
					return;
				}
			}
			if(setsockopt(_serverSocketDescriptor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int32_t)) == -1)
			{
				_out.printError("Error: Could not set socket options.");
				return;
			}
			if(bind(_serverSocketDescriptor, info->ai_addr, info->ai_addrlen) == -1)
			{
				error = errno;
				continue;
			}
			sockaddr_in address;
			socklen_t addrLength = sizeof(address);
			memset(&address, 0, sizeof(address));
			getsockname(_serverSocketDescriptor, (sockaddr*)(&address), &addrLength);
			_id = std::string("binary://127.0.0.1:") + std::to_string(ntohs(address.sin_port));
			_out.printInfo("Info: RPC Server started listening.");
			bound = true;
			break;
		}
		freeaddrinfo(serverInfo);
		if(!bound)
		{
			shutdown(_serverSocketDescriptor, 0);
			_out.printCritical("Error: Server could not start listening: " + std::string(strerror(error)));
			return;
		}
		if(_serverSocketDescriptor == -1 || !bound || listen(_serverSocketDescriptor, _backlog) == -1)
		{
			shutdown(_serverSocketDescriptor, 0);
			_out.printCritical("Error: Server could not start listening on port: " + std::string(strerror(errno)));
			return;
		}
    }
    catch(const std::exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(Exception& ex)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
    	_out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}
}
