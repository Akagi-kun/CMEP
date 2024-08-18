#pragma once

#include "lua.hpp"

#include <string>
#include <unordered_map>

namespace Engine::Scripting::Mappings
{
	namespace Functions
	{
		int PrintReplace(lua_State* state);
	} // namespace Functions

	extern std::unordered_map<std::string, lua_CFunction> mappings;

} // namespace Engine::Scripting::Mappings
