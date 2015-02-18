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

#ifndef RPCMETHODS_H_
#define RPCMETHODS_H_

#include "RPCServer.h"
#include "RPCMethod.h"
#include "Variable.h"

#include <vector>
#include <memory>
#include <cstdlib>

namespace HgAddonLib
{

class RPCSystemListMethods : public RPCMethod
{
public:
	RPCSystemListMethods()
	{
		setHelp("Lists all XML RPC methods.");
		addSignature(VariableType::rpcArray, std::vector<VariableType>());
	}
	std::shared_ptr<Variable> invoke(std::shared_ptr<std::vector<std::shared_ptr<Variable>>> parameters);
};

class RPCSystemMethodHelp : public RPCMethod
{
public:
	RPCSystemMethodHelp()
	{
		setHelp("Returns a description of the method.");
		addSignature(VariableType::rpcString, std::vector<VariableType>{VariableType::rpcString});
	}
	std::shared_ptr<Variable> invoke(std::shared_ptr<std::vector<std::shared_ptr<Variable>>> parameters);
};

class RPCSystemMethodSignature : public RPCMethod
{
public:
	RPCSystemMethodSignature()
	{
		setHelp("Returns the method's signature.");
		addSignature(VariableType::rpcArray, std::vector<VariableType>{VariableType::rpcString});
	}
	std::shared_ptr<Variable> invoke(std::shared_ptr<std::vector<std::shared_ptr<Variable>>> parameters);
};

class RPCSystemMulticall : public RPCMethod
{
public:
	RPCSystemMulticall()
	{
		setHelp("Calls multiple XML RPC methods.");
		addSignature(VariableType::rpcArray, std::vector<VariableType>{VariableType::rpcArray});
	}
	std::shared_ptr<Variable> invoke(std::shared_ptr<std::vector<std::shared_ptr<Variable>>> parameters);
};

class RPCEvent : public RPCMethod
{
public:
	RPCEvent(Base* base)
	{
		_base = base;
		addSignature(VariableType::rpcArray, std::vector<VariableType>{VariableType::rpcArray});
	}
	std::shared_ptr<Variable> invoke(std::shared_ptr<std::vector<std::shared_ptr<Variable>>> parameters);
protected:
	Base* _base = nullptr;
};

}
#endif
