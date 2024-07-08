#include "Scripting/API/LuaFactories.hpp"

#include "Scripting/API/AssetManager_API.hpp"
#include "Scripting/API/Engine_API.hpp"
#include "Scripting/API/Object_API.hpp"
#include "Scripting/API/SceneManager_API.hpp"

#include "lua.h"

#include <memory>
#include <utility>

namespace Engine::Scripting::API::LuaObjectFactories
{
	void SceneManagerFactory(lua_State* state, std::weak_ptr<SceneManager> scene_manager_ptr)
	{
		// Generate SceneManager table
		lua_createtable(state, 0, static_cast<int>(scene_manager_mappings.size() + 1));

		// Add SceneManager mappings
		for (const auto& mapping : scene_manager_mappings)
		{
			lua_pushcfunction(state, mapping.second);
			lua_setfield(state, -2, mapping.first.c_str());
		}

		// Add SceneManager pointer
		void* ptr_obj = lua_newuserdata(state, sizeof(std::weak_ptr<SceneManager>));
		new (ptr_obj) std::weak_ptr<SceneManager>(std::move(scene_manager_ptr));

		lua_setfield(state, -2, "_smart_ptr");
	}

	void ObjectFactory(lua_State* state, Object* object_ptr)
	{
		// Generate Object table
		lua_createtable(state, 0, static_cast<int>(object_mappings.size() + 2));

		// Object mappings
		for (const auto& mapping : object_mappings)
		{
			lua_pushcfunction(state, mapping.second);
			lua_setfield(state, -2, mapping.first.c_str());
		}

		// Add Object pointer
		lua_pushlightuserdata(state, object_ptr);
		lua_setfield(state, -2, "_pointer");

		// Generate renderer table
		lua_newtable(state);
		lua_pushlightuserdata(state, object_ptr->GetRenderer());
		lua_setfield(state, -2, "_pointer");
		lua_setfield(state, -2, "renderer");
	}

	void AssetManagerFactory(lua_State* state, std::weak_ptr<AssetManager> asset_manager_ptr)
	{
		// Generate AssetManager table
		lua_createtable(state, 0, static_cast<int>(asset_manager_mappings.size() + 1));

		// AssetManager mappings
		for (const auto& mapping : asset_manager_mappings)
		{
			lua_pushcfunction(state, mapping.second);
			lua_setfield(state, -2, mapping.first.c_str());
		}

		// Add AssetManager pointer
		void* ptr_obj = lua_newuserdata(state, sizeof(std::weak_ptr<AssetManager>));
		new (ptr_obj) std::weak_ptr<AssetManager>(std::move(asset_manager_ptr));
		lua_setfield(state, -2, "_smart_ptr");
	}

	void EngineFactory(lua_State* state, Engine* engine_ptr)
	{
		// Generate Engine table
		lua_createtable(state, 0, static_cast<int>(engine_mappings.size() + 1));

		// Engine mappings
		for (const auto& mapping : engine_mappings)
		{
			lua_pushcfunction(state, mapping.second);
			lua_setfield(state, -2, mapping.first.c_str());
		}

		// Add Engine pointer
		lua_pushlightuserdata(state, engine_ptr);
		lua_setfield(state, -2, "_pointer");
	}

	std::weak_ptr<Logging::Logger> MetaLoggerFactory(lua_State* state)
	{
		lua_getglobal(state, "cmepmeta");
		lua_getfield(state, -1, "logger");
		lua_getfield(state, -1, "_smart_ptr");
		std::weak_ptr<Logging::Logger> logger = *static_cast<std::weak_ptr<Logging::Logger>*>(lua_touserdata(state, -1)
		);

		return logger;
	}
} // namespace Engine::Scripting::API::LuaObjectFactories
