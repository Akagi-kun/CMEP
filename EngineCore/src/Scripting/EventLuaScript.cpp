#include "Scripting/EventLuaScript.hpp"

#include "Scripting/API/LuaFactories.hpp"
#include "Scripting/Utility.hpp"

#include "EventHandling.hpp"

#include <string>

namespace Engine::Scripting
{
	using Scripting::API::LuaFactories::engineFactory;

	void EventLuaScript::initializeCall(const std::string& function)
	{
		// Clear stack
		lua_settop(state, 0);

		// Push error handler
		lua_pushcfunction(state, Utility::luaErrorHandler);

		// Get start function
		lua_getglobal(state, function.c_str());
	}

	int EventLuaScript::internalCall(const std::string& function, void* data)
	{
		initializeCall(function);

		const auto* event = static_cast<EventHandling::Event*>(data);

		// Event table
		lua_createtable(state, 0, 4);
		lua_pushnumber(state, event->delta_time);
		lua_setfield(state, -2, "deltaTime");

		lua_pushinteger(state, event->keycode);
		lua_setfield(state, -2, "keycode");

		engineFactory(state, event->raised_from);
		lua_setfield(state, -2, "engine");

		// Mouse table
		lua_newtable(state);
		lua_pushnumber(state, event->mouse.x);
		lua_setfield(state, -2, "x");
		lua_pushnumber(state, event->mouse.y);
		lua_setfield(state, -2, "y");
		lua_setfield(state, -2, "mouse");

		// Call into script
		// Stack content: [
		//		...,
		//		(-3, func)LuaErrorHandler,
		//		(-2, func)StartFunction,
		//		(-1, table)Event
		//	] <- stack top is here
		//
		return lua_pcall(state, 1, 1, -3);
	}
} // namespace Engine::Scripting
