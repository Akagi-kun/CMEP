#pragma once

#include "AssetManager.hpp"
#include "SceneManager.hpp"

namespace Engine::Scripting::API::LuaObjectFactories
{
	void AssetManagerFactory(lua_State* state, std::weak_ptr<AssetManager> asset_manager_ptr);
	void SceneManagerFactory(lua_State* state, std::weak_ptr<SceneManager> scene_manager);
	void ObjectFactory(lua_State* state, Object* object_ptr);
	void EngineFactory(lua_State* state, Engine* engine_ptr);

	std::weak_ptr<Logging::Logger> MetaLoggerFactory(lua_State* state);
} // namespace Engine::Scripting::API::LuaObjectFactories