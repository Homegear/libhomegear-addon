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


#ifndef RPCSERVER_H_
#define RPCSERVER_H_

#include "RPCMethod.h"
#include "Output.h"
#include "Encoding/RPCDecoder.h"
#include "Encoding/RPCEncoder.h"
#include "SocketOperations.h"
#include "Base.h"

#include <thread>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <iterator>
#include <sstream>
#include <utility>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <errno.h>

namespace HgAddonLib
{
	class RPCServer {
		public:
			RPCServer();
			virtual ~RPCServer();

			void start(Base* base, uint64_t myPeerId);
			void stop();
			std::map<std::string, RPCMethod>* getMethods() { return &_rpcMethods; }
			std::shared_ptr<Variable> callMethod(std::string& methodName, std::shared_ptr<Variable>& parameters);
			std::string getId() { return (_serverSocketDescriptor != -1) ? _id : ""; }

			void addPeers(std::vector<uint64_t>& peerIds);
			void removePeers(std::vector<uint64_t>& peerIds);
		protected:
		private:
			Output _out;
			bool _stopServer = false;
			std::thread _mainThread;
			int32_t _backlog = 2;
			int32_t _serverSocketDescriptor = -1;
			std::map<std::string, RPCMethod> _rpcMethods;
			RPCDecoder _rpcDecoder;
			RPCEncoder _rpcEncoder;
			std::string _id;
			int32_t _lastInit = 0;
			int32_t _lastKeepAlive = 0;
			uint64_t _myPeerId = 0;
			std::mutex _subscribedPeersMutex;
			std::set<uint64_t> _subscribedPeers;

			void getSocketDescriptor();
			int32_t getClientSocketDescriptor();
			void mainThread();
			void readClient(SocketOperations& socket);
			void sendRPCResponseToClient(SocketOperations& socket, std::shared_ptr<Variable> error);
			void sendRPCResponseToClient(SocketOperations& socket, std::vector<char>& data);
			void analyzeRPC(SocketOperations& socket, std::vector<char>& packet);
			void callMethod(SocketOperations& socket, std::string methodName, std::shared_ptr<std::vector<std::shared_ptr<Variable>>> parameters);
			void registerMethods(Base* base);
			void keepAlive();
			void sendInit();
	};
}
#endif
