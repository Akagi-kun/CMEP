#pragma once

#include "lua.hpp"

#include <string>
#include <unordered_map>

namespace Engine::Scripting::API
{
	/**
	 * Function to replace the default Lua print() function
	 *
	 * Expects upvalue at index 1 to point to a Logger instance
	 */
	int printReplace(lua_State* state);

	extern std::unordered_map<std::string, const lua_CFunction> global_mappings;

} // namespace Engine::Scripting::API
