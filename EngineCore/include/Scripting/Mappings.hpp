#pragma once

// #include "lualib/lua.hpp"
#include "lua.hpp"

#include <string>
#include <unordered_map>

namespace Engine::Scripting::Mappings
{
	namespace Functions
	{
		int MetaLoggerSimpleLog(lua_State* state);
	} // namespace Functions

	extern std::unordered_map<std::string, lua_CFunction> scene_manager_mappings;
	extern std::unordered_map<std::string, lua_CFunction> object_mappings;

	[[deprecated]]
	extern std::unordered_map<std::string, lua_CFunction> mappings;

} // namespace Engine::Scripting::Mappings
