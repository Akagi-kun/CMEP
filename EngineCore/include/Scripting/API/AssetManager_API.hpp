#pragma once

// #include "Scripting/lualib/lua.hpp"
#include "lua.hpp"

#include <string>
#include <unordered_map>

namespace Engine::Scripting::API
{
	extern std::unordered_map<std::string, const lua_CFunction> asset_manager_mappings;
}
