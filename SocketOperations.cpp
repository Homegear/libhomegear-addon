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

#include "SocketOperations.h"
#include "GD.h"

namespace HgAddonLib
{
SocketOperations::SocketOperations()
{
	_autoConnect = false;
}

SocketOperations::SocketOperations(int32_t socketDescriptor)
{
	_autoConnect = false;
	if(socketDescriptor) _socketDescriptor = socketDescriptor;
}

SocketOperations::SocketOperations(std::string hostname, std::string port)
{
	signal(SIGPIPE, SIG_IGN);

	_hostname = hostname;
	_port = port;
}

SocketOperations::~SocketOperations()
{
}

void SocketOperations::open()
{
	if(_socketDescriptor < 0) getSocketDescriptor();
	else if(!connected())
	{
		close();
		getSocketDescriptor();
	}
}

void SocketOperations::autoConnect()
{
	if(!_autoConnect) return;
	if(_socketDescriptor < 0) getSocketDescriptor();
	else if(!connected())
	{
		close();
		getSocketDescriptor();
	}
}

void SocketOperations::close()
{
	if(_socketDescriptor < 0) return;
	::close(_socketDescriptor);
	_socketDescriptor = -1;
}

void SocketOperations::shutdown()
{
	if(_socketDescriptor < 0) return;
	::shutdown(_socketDescriptor, 0);
	_socketDescriptor = -1;
}


int32_t SocketOperations::proofread(char* buffer, int32_t bufferSize)
{
	if(!_socketDescriptor) throw SocketOperationException("Socket descriptor is nullptr.");
	if(!connected()) autoConnect();
	//Timeout needs to be set every time, so don't put it outside of the while loop
	timeval timeout;
	int32_t seconds = _readTimeout / 1000000;
	timeout.tv_sec = seconds;
	timeout.tv_usec = _readTimeout - (1000000 * seconds);
	fd_set readFileDescriptor;
	FD_ZERO(&readFileDescriptor);
	int32_t nfds = _socketDescriptor + 1;
	if(nfds <= 0)
	{
		throw SocketClosedException("Connection closed (1).");
	}
	FD_SET(_socketDescriptor, &readFileDescriptor);
	int32_t bytesRead = select(nfds, &readFileDescriptor, NULL, NULL, &timeout);
	if(bytesRead == 0) throw SocketTimeOutException("Reading from socket timed out.");
	if(bytesRead != 1) throw SocketClosedException("Connection closed (2).");
	do
	{
		bytesRead = read(_socketDescriptor, buffer, bufferSize);
	} while(bytesRead < 0 && errno == EAGAIN);
	if(bytesRead <= 0) throw SocketClosedException("Connection to client number closed (3).");
	return bytesRead;
}

int32_t SocketOperations::proofwrite(const std::shared_ptr<std::vector<char>> data)
{
	if(!connected()) autoConnect();
	if(!data || data->empty()) return 0;
	return proofwrite(*data);
}

int32_t SocketOperations::proofwrite(const std::vector<char>& data)
{

	if(!_socketDescriptor) throw SocketOperationException("Socket descriptor is nullptr.");
	if(!connected()) autoConnect();
	if(data.empty()) return 0;
	if(data.size() > 10485760) throw SocketDataLimitException("Data size is larger than 10 MiB.");
	GD::out.printDebug("Debug: ... data size is " + std::to_string(data.size()), 6);

	int32_t totalBytesWritten = 0;
	while (totalBytesWritten < (signed)data.size())
	{
		timeval timeout;
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
		fd_set writeFileDescriptor;
		FD_ZERO(&writeFileDescriptor);
		int32_t nfds = _socketDescriptor + 1;
		if(nfds <= 0)
		{
			throw SocketClosedException("Connection to client number closed (4).");
		}
		FD_SET(_socketDescriptor, &writeFileDescriptor);
		int32_t readyFds = select(nfds, NULL, &writeFileDescriptor, NULL, &timeout);
		if(readyFds == 0) throw SocketTimeOutException("Writing to socket timed out.");
		if(readyFds != 1) throw SocketClosedException("Connection to client number closed (5).");

		int32_t bytesWritten = send(_socketDescriptor, &data.at(totalBytesWritten), data.size() - totalBytesWritten, MSG_NOSIGNAL);
		if(bytesWritten <= 0)
		{
			close();
			throw SocketOperationException(strerror(errno));
		}
		totalBytesWritten += bytesWritten;
	}
	return totalBytesWritten;
}

int32_t SocketOperations::proofwrite(const std::string& data)
{

	GD::out.printDebug("Debug: Calling proofwrite ...", 6);
	if(!_socketDescriptor) throw SocketOperationException("Socket descriptor is nullptr.");
	if(!connected()) autoConnect();
	if(data.empty()) return 0;
	if(data.size() > 10485760) throw SocketDataLimitException("Data size is larger than 10 MiB.");
	GD::out.printDebug("Debug: ... data size is " + std::to_string(data.size()), 6);

	int32_t bytesSentSoFar = 0;
	while (bytesSentSoFar < (signed)data.size())
	{
		timeval timeout;
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
		fd_set writeFileDescriptor;
		FD_ZERO(&writeFileDescriptor);
		int32_t nfds = _socketDescriptor + 1;
		if(nfds <= 0)
		{
			throw SocketClosedException("Connection to client number closed (6).");
		}
		FD_SET(_socketDescriptor, &writeFileDescriptor);
		int32_t readyFds = select(nfds, NULL, &writeFileDescriptor, NULL, &timeout);
		if(readyFds == 0) throw SocketTimeOutException("Writing to socket timed out.");
		if(readyFds != 1) throw SocketClosedException("Connection to client number closed (7).");

		int32_t bytesToSend = data.size() - bytesSentSoFar;
		int32_t bytesSentInStep = send(_socketDescriptor, &data.at(bytesSentSoFar), bytesToSend, MSG_NOSIGNAL);
		if(bytesSentInStep <= 0)
		{
			GD::out.printDebug("Debug: ... exception at " + std::to_string(bytesSentSoFar) + " error is " + strerror(errno));
			close();
			throw SocketOperationException(strerror(errno));
		}
		bytesSentSoFar += bytesSentInStep;
	}
	GD::out.printDebug("Debug: ... sent " + std::to_string(bytesSentSoFar), 6);
	return bytesSentSoFar;
}

bool SocketOperations::connected()
{
	if(_socketDescriptor == -1) return false;
	char buffer[1];
	if(recv(_socketDescriptor, buffer, sizeof(buffer), MSG_PEEK | MSG_DONTWAIT) == 0) return false;
	return true;
}

void SocketOperations::getSocketDescriptor()
{
	GD::out.printDebug("Debug: Calling getFileDescriptor...");
	shutdown();

	getConnection();
	if(_socketDescriptor < 0) throw SocketOperationException("Could not connect to server.");
}

void SocketOperations::getConnection()
{
	if(_hostname.empty()) throw SocketInvalidParametersException("Hostname is empty");
	if(_port.empty()) throw SocketInvalidParametersException("Port is empty");

	GD::out.printInfo("Info: Connecting to host " + _hostname + " on port " + _port + "...");

	//Retry for two minutes
	for(uint32_t i = 0; i < 6; ++i)
	{
		struct addrinfo *serverInfo = nullptr;
		struct addrinfo hostInfo;
		memset(&hostInfo, 0, sizeof hostInfo);

		hostInfo.ai_family = AF_UNSPEC;
		hostInfo.ai_socktype = SOCK_STREAM;

		if(getaddrinfo(_hostname.c_str(), _port.c_str(), &hostInfo, &serverInfo) != 0)
		{
			freeaddrinfo(serverInfo);
			throw SocketOperationException("Could not get address information: " + std::string(strerror(errno)));
		}

		char ipStringBuffer[INET6_ADDRSTRLEN];
		if (serverInfo->ai_family == AF_INET) {
			struct sockaddr_in *s = (struct sockaddr_in *)serverInfo->ai_addr;
			inet_ntop(AF_INET, &s->sin_addr, ipStringBuffer, sizeof(ipStringBuffer));
		} else { // AF_INET6
			struct sockaddr_in6 *s = (struct sockaddr_in6 *)serverInfo->ai_addr;
			inet_ntop(AF_INET6, &s->sin6_addr, ipStringBuffer, sizeof(ipStringBuffer));
		}
		std::string ipAddress = std::string(&ipStringBuffer[0]);

		_socketDescriptor = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
		if(_socketDescriptor == -1)
		{
			freeaddrinfo(serverInfo);
			throw SocketOperationException("Could not create socket for server " + ipAddress + " on port " + _port + ": " + strerror(errno));
		}
		int32_t optValue = 1;
		if(setsockopt(_socketDescriptor, SOL_SOCKET, SO_KEEPALIVE, (void*)&optValue, sizeof(int32_t)) == -1)
		{
			freeaddrinfo(serverInfo);
			shutdown();
			throw SocketOperationException("Could not set socket options for server " + ipAddress + " on port " + _port + ": " + strerror(errno));
		}
		optValue = 30;
		if(setsockopt(_socketDescriptor, SOL_TCP, TCP_KEEPIDLE, (void*)&optValue, sizeof(int32_t)) == -1)
		{
			freeaddrinfo(serverInfo);
			shutdown();
			throw SocketOperationException("Could not set socket options for server " + ipAddress + " on port " + _port + ": " + strerror(errno));
		}
		optValue = 4;
		if(setsockopt(_socketDescriptor, SOL_TCP, TCP_KEEPCNT, (void*)&optValue, sizeof(int32_t)) == -1)
		{
			freeaddrinfo(serverInfo);
			shutdown();
			throw SocketOperationException("Could not set socket options for server " + ipAddress + " on port " + _port + ": " + strerror(errno));
		}
		optValue = 15;
		if(setsockopt(_socketDescriptor, SOL_TCP, TCP_KEEPINTVL, (void*)&optValue, sizeof(int32_t)) == -1)
		{
			freeaddrinfo(serverInfo);
			shutdown();
			throw SocketOperationException("Could not set socket options for server " + ipAddress + " on port " + _port + ": " + strerror(errno));
		}

		if(!(fcntl(_socketDescriptor, F_GETFL) & O_NONBLOCK))
		{
			if(fcntl(_socketDescriptor, F_SETFL, fcntl(_socketDescriptor, F_GETFL) | O_NONBLOCK) < 0)
			{
				freeaddrinfo(serverInfo);
				shutdown();
				throw SocketOperationException("Could not set socket options for server " + ipAddress + " on port " + _port + ": " + strerror(errno));
			}
		}

		int32_t connectResult;
		if((connectResult = connect(_socketDescriptor, serverInfo->ai_addr, serverInfo->ai_addrlen)) == -1 && errno != EINPROGRESS)
		{
			if(i < 5)
			{
				freeaddrinfo(serverInfo);
				shutdown();
				std::this_thread::sleep_for(std::chrono::milliseconds(3000));
				continue;
			}
			else
			{
				freeaddrinfo(serverInfo);
				shutdown();
				throw SocketTimeOutException("Connecting to server " + ipAddress + " on port " + _port + " timed out: " + strerror(errno));
			}
		}
		freeaddrinfo(serverInfo);

		if(connectResult != 0) //We have to wait for the connection
		{
			pollfd pollstruct
			{
				(int)_socketDescriptor,
				(short)(POLLIN | POLLOUT | POLLERR),
				(short)0
			};

			int32_t pollResult = poll(&pollstruct, 1, 5000);
			if(pollResult < 0 || (pollstruct.revents & POLLERR))
			{
				if(i < 5)
				{
					shutdown();
					std::this_thread::sleep_for(std::chrono::milliseconds(3000));
					continue;
				}
				else
				{
					shutdown();
					throw SocketTimeOutException("Could not connect to server " + ipAddress + " on port " + _port + ". Poll failed with error code: " + std::to_string(pollResult) + ".");
				}
			}
			else if(pollResult > 0)
			{
				socklen_t resultLength = sizeof(connectResult);
				if(getsockopt(_socketDescriptor, SOL_SOCKET, SO_ERROR, &connectResult, &resultLength) < 0)
				{
					shutdown();
					throw SocketOperationException("Could not connect to server " + ipAddress + " on port " + _port + ": " + strerror(errno) + ".");
				}
				break;
			}
			else if(pollResult == 0)
			{
				if(i < 5)
				{
					shutdown();
					continue;
				}
				else
				{
					shutdown();
					throw SocketTimeOutException("Connecting to server " + ipAddress + " on port " + _port + " timed out.");
				}
			}
		}
	}
	GD::out.printInfo("Info: Connected to host " + _hostname + " on port " + _port + ".");
}
}
