#include "Scripting/API/LuaFactories.hpp"

#include "Assets/AssetManager.hpp"

#include "Scripting/API/AssetManager_API.hpp"
#include "Scripting/API/Engine_API.hpp"
#include "Scripting/API/Object_API.hpp"
#include "Scripting/API/SceneManager_API.hpp"
#include "Scripting/API/Scene_API.hpp"

#include "Engine.hpp"
#include "Scene.hpp"
#include "SceneManager.hpp"
#include "SceneObject.hpp"
#include "lua.hpp"

#include <cassert>
#include <string>
#include <unordered_map>

namespace Engine::Scripting::API::LuaFactories
{
	/// @cond LUA_API
	namespace
	{
		int mappingTrampoline(lua_State* state)
		{
			// arg1 = table
			// arg2 = index

			const char* index = lua_tostring(state, 2);

			auto* mapping_upvalue =
				static_cast<std::unordered_map<std::string, const lua_CFunction>*>(
					lua_touserdata(state, lua_upvalueindex(1))
				);

			auto found = mapping_upvalue->find(index);
			if (found != mapping_upvalue->end())
			{
				lua_pop(state, 1);
				lua_pushcclosure(state, found->second, 0);

				return 1;
			}

			return luaL_error(state, "Mapping '%s' not found by __index metafn!", index);
		}

		int createMappingTrampoline(
			lua_State*											  state,
			std::unordered_map<std::string, const lua_CFunction>* mapping
		)
		{
			// Index -1 must contain a table
			assert(lua_istable(state, -1) == true);
			// (1)

			lua_createtable(state, 0, 1);
			// +1 (2)

			lua_pushlightuserdata(state, mapping);
			// +1 (4)

			lua_pushcclosure(state, mappingTrampoline, 1);
			// -1 +1 (3)

			lua_setfield(state, -2, "__index");
			// -1 (2)

			lua_setmetatable(state, -2);
			// -1 (1)

			return 1;
		}
	} // namespace

	template <>
	void templatedFactory<SceneManager>(lua_State* state, SceneManager* object_ptr)
	{
		// Generate SceneManager table
		lua_createtable(state, 0, 2);

		createMappingTrampoline(state, &scene_manager_mappings);

		// Add SceneManager pointer
		lua_pushlightuserdata(state, object_ptr);
		lua_setfield(state, -2, "_ptr");
	}

	template <>
	void templatedFactory<Scene>(lua_State* state, Scene* object_ptr)
	{
		// Generate Scene table
		lua_createtable(state, 0, 2);

		createMappingTrampoline(state, &scene_mappings);

		// Scene pointer
		lua_pushlightuserdata(state, object_ptr);
		lua_setfield(state, -2, "_ptr");
	}

	template <>
	void templatedFactory<SceneObject>(lua_State* state, SceneObject* object_ptr)
	{
		// Generate Object table
		lua_createtable(state, 0, 4);

		createMappingTrampoline(state, &object_mappings);

		// Object pointer
		lua_pushlightuserdata(state, object_ptr);
		lua_setfield(state, -2, "_ptr");

		// Renderer
		lua_createtable(state, 0, 1);
		lua_pushlightuserdata(state, object_ptr->getRenderer());
		lua_setfield(state, -2, "_ptr");
		lua_setfield(state, -2, "renderer");

		// MeshBuilder
		lua_createtable(state, 0, 1);
		lua_pushlightuserdata(state, object_ptr->getMeshBuilder());
		lua_setfield(state, -2, "_ptr");
		lua_setfield(state, -2, "meshbuilder");
	}

	template <>
	void templatedFactory<AssetManager>(lua_State* state, AssetManager* object_ptr)
	{
		// Generate AssetManager table
		lua_createtable(state, 0, 2);

		createMappingTrampoline(state, &asset_manager_mappings);

		// AssetManager pointer
		lua_pushlightuserdata(state, object_ptr);
		lua_setfield(state, -2, "_ptr");
	}

	template <>
	void templatedFactory<Engine>(lua_State* state, Engine* object_ptr)
	{
		// Generate Engine table
		lua_createtable(state, 0, 2);

		createMappingTrampoline(state, &engine_mappings);

		// Engine pointer
		lua_pushlightuserdata(state, object_ptr);
		lua_setfield(state, -2, "_ptr");
	}
	/// @endcond
} // namespace Engine::Scripting::API::LuaFactories
