#pragma once

#include "Assets/AssetManager.hpp"

#include "Logging/Logging.hpp"

#include "SceneManager.hpp"

#include <memory>

namespace Engine::Scripting::API::LuaFactories
{
	void assetManagerFactory(lua_State* state, std::weak_ptr<AssetManager> asset_manager_ptr);
	void sceneManagerFactory(lua_State* state, SceneManager* scene_manager);
	void sceneFactory(lua_State* state, Scene* scene_ptr);
	void objectFactory(lua_State* state, Object* object_ptr);
	void engineFactory(lua_State* state, Engine* engine_ptr);

	std::weak_ptr<Logging::Logger> loggerObjectFactory(lua_State* state);
} // namespace Engine::Scripting::API::LuaFactories
