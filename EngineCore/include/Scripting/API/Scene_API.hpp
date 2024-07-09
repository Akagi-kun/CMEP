#pragma once

#include "lua.hpp"

#include <string>
#include <unordered_map>

namespace Engine::Scripting::API
{
	extern const std::unordered_map<std::string, lua_CFunction> scene_mappings;
}
