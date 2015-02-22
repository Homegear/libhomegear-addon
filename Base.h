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

#ifndef HGADDONBASE_H_
#define HGADDONBASE_H_

#include <memory>
#include <list>

#include "Variable.h"

namespace HgAddonLib
{

#define GETRPCCLIENTPARAMETERS(_1,_2,_3,_4,_5,_6,_7,NAME,...) NAME
#define RPCCLIENTPARAMETERS(...) GETRPCCLIENTPARAMETERS(__VA_ARGS__, RPCCLIENTPARAMETERS7, RPCCLIENTPARAMETERS6, RPCCLIENTPARAMETERS5, RPCCLIENTPARAMETERS4, RPCCLIENTPARAMETERS3, RPCCLIENTPARAMETERS2, RPCCLIENTPARAMETERS1)(__VA_ARGS__)
#define RPCCLIENTPARAMETERS1(p1) PRPCList(new RPCList { PVariable(new Variable(p1)) })
#define RPCCLIENTPARAMETERS2(p1, p2) PRPCList(new RPCList { PVariable(new Variable(p1)), PVariable(new Variable(p2)) })
#define RPCCLIENTPARAMETERS3(p1, p2, p3) PRPCList(new RPCList { PVariable(new Variable(p1)), PVariable(new Variable(p2)), PVariable(new Variable(p3)) })
#define RPCCLIENTPARAMETERS4(p1, p2, p3, p4) PRPCList(new RPCList { PVariable(new Variable(p1)), PVariable(new Variable(p2)), PVariable(new Variable(p3)), PVariable(new Variable(p4)) })
#define RPCCLIENTPARAMETERS5(p1, p2, p3, p4, p5) PRPCList(new RPCList { PVariable(new Variable(p1)), PVariable(new Variable(p2)), PVariable(new Variable(p3)), PVariable(new Variable(p4)), PVariable(new Variable(p5)) })
#define RPCCLIENTPARAMETERS6(p1, p2, p3, p4, p5, p6) PRPCList(new RPCList { PVariable(new Variable(p1)), PVariable(new Variable(p2)), PVariable(new Variable(p3)), PVariable(new Variable(p4)), PVariable(new Variable(p5)), PVariable(new Variable(p6)) })
#define RPCCLIENTPARAMETERS7(p1, p2, p3, p4, p5, p6, p7) PRPCList(new RPCList { PVariable(new Variable(p1)), PVariable(new Variable(p2)), PVariable(new Variable(p3)), PVariable(new Variable(p4)), PVariable(new Variable(p5)), PVariable(new Variable(p6)), PVariable(new Variable(p7)) })

class Base
{
public:
	/**
	 * Constructor.
	 *
	 * @param homegearPort The port Homegear's RPC server listens on.
	 * @param myPeerId The peer id of the addon. Can be "0". If "0" no events are received from Homegear before "addPeer" or "addPeers" is called.
	 * @param debugLevel The debug level. Possible values: 1 (critical), 2 (error), 3 (warning), 4 (info), 5 (debug)
	 */
	Base(int32_t homegearPort, uint64_t myPeerId, int32_t debugLevel);

	/**
	 * Destructor
	 */
	virtual ~Base();

	/**
	 * Subscribe to Homegear events from the provided peer.
	 *
	 * @param peerId The id of the peer to subscribe to.
	 */
	virtual void addPeer(uint64_t peerId);

	/**
	 * Subscribe to Homegear events from the provided peers.
	 *
	 * @param peerIds Array of peer ids to subscribe to.
	 */
	virtual void addPeers(std::vector<uint64_t> peerId);

	/**
	 * Removes the subscription of events from the provided peer.
	 *
	 * @param peerId The id of the peer to remove the subscription for.
	 */
	virtual void removePeer(uint64_t peerId);

	/**
	 * Removes the subscription of events from the provided peers.
	 *
	 * @param peerIds The ids of the peers to remove the subscription for.
	 */
	virtual void removePeers(std::vector<uint64_t> peerIds);

	/**
	 * With this method you can call RPC functions in Homegear.
	 *
	 * @see <a href="https://www.homegear.eu/index.php/Homegear_Reference">Homegear Reference</a>
	 * @param methodName The name of the RPC method. See the Homegear reference for more information.
	 * @param parameters List with the parameters to pass to the RPC function. You can use the macro RPCCLIENTPARAMETERS(parameter1, parameter2, ...) for easier usage. E. g.: invoke("getValue", RPCCLIENTPARAMETERS(4, 1, std::string("STATE")));
	 * @return Returns the result of the RPC call received from Homegear.
	 */
	virtual PVariable invoke(std::string methodName, PRPCList parameters = PRPCList());

	/**
	 * Homegear calls this method when a device is deleted. Overload it when needed.
	 *
	 * @param peerId The id of the deleted peer.
	 */
	virtual void deleteDevice(uint64_t peerId) {}

	/**
	 * Homegear calls this method when an error occurs. Overload it when needed.
	 *
	 * @param level The level of the error 1 (critical), 2 (error) or 3 (warning)
	 * @param message The error message.
	 */
	virtual void error(int32_t level, std::string message) {}

	/**
	 * Homegear calls this method when a device parameter changes. Overload it when needed.
	 *
	 * @param peerId The id of the peer whose variable changed.
	 * @param channel The channel of the changed variable.
	 * @param parameter The name of the changed variable.
	 * @param value The new value of the variable (can be the same as before).
	 */
	virtual void event(uint64_t peerId, int32_t channel, std::string parameter, PVariable value) {}

	/**
	 * Homegear calls this method when a new device was added. Overload it when needed.
	 *
	 * @param peerId The id of the new peer.
	 */
	virtual void newDevice(uint64_t peerId) {}

	/**
	 * Homegear calls this method when a device's configuration has changed. Overload it when needed.
	 *
	 * @param peerId The id of the updated peer.
	 * @param channel The index of the updated channel (or -1 if all device data needs to be updated).
	 * @param flags One of: 0: Change to device's or channel's configuration (including link configuration), 1: Change to device's or channel's device links, 2: Change to device's or channel's teams
	 */
	virtual void updateDevice(uint64_t peerId, int32_t channel, int32_t flags) {}
protected:

};

}
#endif
