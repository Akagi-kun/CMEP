#pragma once

#include "SceneManager.hpp"

#include "framework.hpp"

namespace Engine::Scripting::API::LuaObjectFactories
{
	void SceneManagerFactory(lua_State* state, std::weak_ptr<SceneManager> scene_manager);
	void ObjectFactory(lua_State* state, Object* object_ptr);

	std::weak_ptr<Logging::Logger> MetaLogger_Factory(lua_State* state);
}