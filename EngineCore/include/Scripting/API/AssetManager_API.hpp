#pragma once

#include "AssetManager.hpp"

#include "framework.hpp"

namespace Engine::Scripting::API
{
	extern std::unordered_map<std::string, lua_CFunction> assetManager_Mappings;
}