#pragma once

#include "lua.hpp"

#include <string>

namespace Engine::Scripting::Utility
{
	std::string UnwindStack(lua_State* of_state);
	int LuaErrorHandler(lua_State* state);

	void PrintStackContent(lua_State* state);
} // namespace Engine::Scripting::Utility
