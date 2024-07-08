#pragma once

#include "Assets/AssetManager.hpp"

#include "SceneManager.hpp"

namespace Engine::Scripting::API::LuaFactories
{
	void AssetManagerFactory(lua_State* state, std::weak_ptr<AssetManager> asset_manager_ptr);
	void SceneManagerFactory(lua_State* state, SceneManager* scene_manager);
	void SceneFactory(lua_State* state, Scene* scene_ptr);
	void ObjectFactory(lua_State* state, Object* object_ptr);
	void EngineFactory(lua_State* state, Engine* engine_ptr);

	std::weak_ptr<Logging::Logger> MetaLoggerFactory(lua_State* state);
} // namespace Engine::Scripting::API::LuaFactories
