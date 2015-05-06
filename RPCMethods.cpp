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

#include "RPCMethods.h"
#include "GD.h"

namespace HgAddonLib
{

PVariable RPCSystemListMethods::invoke(PRPCArray parameters)
{
	try
	{
		if(parameters->size() > 1) return getError(ParameterError::Enum::wrongCount);

		PVariable methods(new Variable(VariableType::rpcArray));

		for(std::map<std::string, std::unique_ptr<RPCMethod>>::iterator i = GD::rpcServer.getMethods()->begin(); i != GD::rpcServer.getMethods()->end(); ++i)
		{
			methods->arrayValue->push_back(PVariable(new Variable(i->first)));
		}

		return methods;
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
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable RPCSystemMethodHelp::invoke(PRPCArray parameters)
{
	try
	{
		ParameterError::Enum error = checkParameters(parameters, std::vector<VariableType>({ VariableType::rpcString }));
		if(error != ParameterError::Enum::noError) return getError(error);

		if(GD::rpcServer.getMethods()->find(parameters->at(0)->stringValue) == GD::rpcServer.getMethods()->end())
		{
			return Variable::createError(-32602, "Method not found.");
		}

		PVariable help = GD::rpcServer.getMethods()->at(parameters->at(0)->stringValue)->getHelp();

		if(!help) help.reset(new Variable(VariableType::rpcString));

		return help;
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
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable RPCSystemMethodSignature::invoke(PRPCArray parameters)
{
	try
	{
		ParameterError::Enum error = checkParameters(parameters, std::vector<VariableType>({ VariableType::rpcString }));
		if(error != ParameterError::Enum::noError) return getError(error);

		if(GD::rpcServer.getMethods()->find(parameters->at(0)->stringValue) == GD::rpcServer.getMethods()->end())
		{
			return Variable::createError(-32602, "Method not found.");
		}

		PVariable signature = GD::rpcServer.getMethods()->at(parameters->at(0)->stringValue)->getSignature();

		if(!signature) signature.reset(new Variable(VariableType::rpcArray));

		if(signature->arrayValue->empty())
		{
			signature->type = VariableType::rpcString;
			signature->stringValue = "undef";
		}

		return signature;
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
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable RPCSystemMulticall::invoke(PRPCArray parameters)
{
	try
	{
		ParameterError::Enum error = checkParameters(parameters, std::vector<VariableType>({ VariableType::rpcArray }));
		if(error != ParameterError::Enum::noError) return getError(error);

		std::map<std::string, std::unique_ptr<RPCMethod>>* methods = GD::rpcServer.getMethods();
		PVariable returns(new Variable(VariableType::rpcArray));
		for(RPCArray::iterator i = parameters->at(0)->arrayValue->begin(); i != parameters->at(0)->arrayValue->end(); ++i)
		{
			if((*i)->type != VariableType::rpcStruct)
			{
				returns->arrayValue->push_back(Variable::createError(-32602, "Array element is no struct."));
				continue;
			}
			if((*i)->structValue->size() != 2)
			{
				returns->arrayValue->push_back(Variable::createError(-32602, "Struct has wrong size."));
				continue;
			}
			if((*i)->structValue->find("methodName") == (*i)->structValue->end() || (*i)->structValue->at("methodName")->type != VariableType::rpcString)
			{
				returns->arrayValue->push_back(Variable::createError(-32602, "No method name provided."));
				continue;
			}
			if((*i)->structValue->find("params") == (*i)->structValue->end() || (*i)->structValue->at("params")->type != VariableType::rpcArray)
			{
				returns->arrayValue->push_back(Variable::createError(-32602, "No parameters provided."));
				continue;
			}
			std::string methodName = (*i)->structValue->at("methodName")->stringValue;
			PRPCArray parameters = (*i)->structValue->at("params")->arrayValue;

			if(methodName == "system.multicall") returns->arrayValue->push_back(Variable::createError(-32602, "Recursive calls to system.multicall are not allowed."));
			else if(methods->find(methodName) == methods->end()) returns->arrayValue->push_back(Variable::createError(-32601, "Requested method not found."));
			else returns->arrayValue->push_back(methods->at(methodName)->invoke(parameters));
		}

		return returns;
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
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable RPCDeleteDevices::invoke(PRPCArray parameters)
{
	try
	{
		ParameterError::Enum error = checkParameters(parameters, std::vector<VariableType>({ VariableType::rpcString, VariableType::rpcArray }));
		if(error != ParameterError::Enum::noError) return getError(error);

		if(!_base) return PVariable(new Variable());

		for(RPCArray::iterator i = parameters->at(1)->arrayValue->begin(); i != parameters->at(1)->arrayValue->end(); ++i)
		{
			if((*i)->structValue->find("ID") != (*i)->structValue->end()) _base->deleteDevice((*i)->structValue->at("ID")->integerValue);
		}
		return PVariable(new Variable());
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
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable RPCError::invoke(PRPCArray parameters)
{
	try
	{
		ParameterError::Enum error = checkParameters(parameters, std::vector<VariableType>({ VariableType::rpcString, VariableType::rpcInteger, VariableType::rpcString }));
		if(error != ParameterError::Enum::noError) return getError(error);

		if(_base) _base->error(parameters->at(1)->integerValue, parameters->at(2)->stringValue);

		return PVariable(new Variable());
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
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable RPCEvent::invoke(PRPCArray parameters)
{
	try
	{
		ParameterError::Enum error = checkParameters(parameters, std::vector<VariableType>({ VariableType::rpcString, VariableType::rpcInteger, VariableType::rpcInteger, VariableType::rpcString, VariableType::rpcVariant }));
		if(error != ParameterError::Enum::noError) return getError(error);

		if(_base) _base->event(parameters->at(1)->integerValue, parameters->at(2)->integerValue, parameters->at(3)->stringValue, parameters->at(4));

		return PVariable(new Variable());
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
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable RPCNewDevices::invoke(PRPCArray parameters)
{
	try
	{
		ParameterError::Enum error = checkParameters(parameters, std::vector<VariableType>({ VariableType::rpcString, VariableType::rpcArray }));
		if(error != ParameterError::Enum::noError) return getError(error);

		if(!_base) return PVariable(new Variable());

		for(RPCArray::iterator i = parameters->at(1)->arrayValue->begin(); i != parameters->at(1)->arrayValue->end(); ++i)
		{
			if((*i)->structValue->find("ID") != (*i)->structValue->end()) _base->newDevice((*i)->structValue->at("ID")->integerValue);
		}
		return PVariable(new Variable());
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
    return Variable::createError(-32500, "Unknown application error.");
}

PVariable RPCUpdateDevice::invoke(PRPCArray parameters)
{
	try
	{
		ParameterError::Enum error = checkParameters(parameters, std::vector<VariableType>({ VariableType::rpcString, VariableType::rpcInteger, VariableType::rpcInteger, VariableType::rpcInteger }));
		if(error != ParameterError::Enum::noError) return getError(error);

		if(_base) _base->updateDevice(parameters->at(1)->integerValue, parameters->at(2)->integerValue, parameters->at(3)->integerValue);

		return PVariable(new Variable());
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
    return Variable::createError(-32500, "Unknown application error.");
}
}
