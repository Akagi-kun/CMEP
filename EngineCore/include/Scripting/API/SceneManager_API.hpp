#pragma once

#include "Scripting/lualib/lua.hpp"
#include <string>
#include <unordered_map>

namespace Engine::Scripting::API
{
	extern std::unordered_map<std::string, lua_CFunction> scene_manager_mappings;
}