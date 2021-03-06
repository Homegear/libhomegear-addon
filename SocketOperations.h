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

#ifndef SOCKETOPERATIONS_H_
#define SOCKETOPERATIONS_H_

#include "Exception.h"

#include <thread>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <list>
#include <iterator>
#include <sstream>
#include <mutex>
#include <memory>
#include <map>
#include <unordered_map>
#include <utility>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/socket.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>

namespace HgAddonLib
{
class SocketOperationException : public Exception
{
public:
	SocketOperationException(std::string message) : Exception(message) {}
};

class SocketSizeMismatchException : public SocketOperationException
{
public:
	SocketSizeMismatchException(std::string message) : SocketOperationException(message) {}
};

class SocketTimeOutException : public SocketOperationException
{
public:
	SocketTimeOutException(std::string message) : SocketOperationException(message) {}
};

class SocketClosedException : public SocketOperationException
{
public:
	SocketClosedException(std::string message) : SocketOperationException(message) {}
};

class SocketInvalidParametersException : public SocketOperationException
{
public:
	SocketInvalidParametersException(std::string message) : SocketOperationException(message) {}
};

class SocketDataLimitException : public SocketOperationException
{
public:
	SocketDataLimitException(std::string message) : SocketOperationException(message) {}
};

class SocketOperations
{
public:
	SocketOperations();
	SocketOperations(int32_t socketDescriptor);
	SocketOperations(std::string hostname, std::string port);
	virtual ~SocketOperations();

	void setReadTimeout(int64_t timeout) { _readTimeout = timeout; }
	void setAutoConnect(bool autoConnect) { _autoConnect = autoConnect; }
	void setHostname(std::string hostname) { close(); _hostname = hostname; }
	void setPort(std::string port) { close(); _port = port; }

	bool connected();
	int32_t proofread(char* buffer, int32_t bufferSize);
	int32_t proofwrite(const std::shared_ptr<std::vector<char>> data);
	int32_t proofwrite(const std::vector<char>& data);
	int32_t proofwrite(const std::string& data);
	void open();
	void close();
	void shutdown();
protected:
	int64_t _readTimeout = 15000000;
	bool _autoConnect = true;
	std::string _hostname;
	std::string _port;

	int32_t _socketDescriptor = -1;

	void getSocketDescriptor();
	void getConnection();
	void autoConnect();
};

}
#endif
