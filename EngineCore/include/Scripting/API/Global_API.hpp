#pragma once

#include "lua.hpp"

#include <string>
#include <unordered_map>

namespace Engine::Scripting::API
{
	int printReplace(lua_State* state);

	extern std::unordered_map<std::string, const lua_CFunction> global_mappings;

} // namespace Engine::Scripting::API
