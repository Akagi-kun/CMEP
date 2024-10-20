#pragma once

#include "Assets/AssetManager.hpp"

#include "EventHandling.hpp"
#include "Scene.hpp"
#include "SceneManager.hpp"
#include "SceneObject.hpp"

#include <memory>

/**
 * Provides functions that construct a proper table for the specified object
 */
namespace Engine::Scripting::API::LuaFactories
{
	void assetManagerFactory(lua_State* state, std::weak_ptr<AssetManager> asset_manager_ptr);
	void sceneManagerFactory(lua_State* state, SceneManager* scene_manager);
	void sceneFactory(lua_State* state, Scene* scene_ptr);
	void objectFactory(lua_State* state, SceneObject* object_ptr);
	void engineFactory(lua_State* state, Engine* engine_ptr);
} // namespace Engine::Scripting::API::LuaFactories
