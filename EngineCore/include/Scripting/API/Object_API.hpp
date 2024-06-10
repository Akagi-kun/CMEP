#pragma once

#include "framework.hpp"

namespace Engine::Scripting::API
{
	extern std::unordered_map<std::string, lua_CFunction> object_mappings;
}