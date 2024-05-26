#pragma once

#include "SceneManager.hpp"

#include "framework.hpp"

namespace Engine::Scripting::API
{
	extern std::unordered_map<std::string, lua_CFunction> sceneManager_Mappings;
}