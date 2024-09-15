#include "Scripting/API/LuaFactories.hpp"

#include "Assets/AssetManager.hpp"

#include "Scripting/API/AssetManager_API.hpp"
#include "Scripting/API/Engine_API.hpp"
#include "Scripting/API/Object_API.hpp"
#include "Scripting/API/SceneManager_API.hpp"
#include "Scripting/API/Scene_API.hpp"

#include "Engine.hpp"
#include "Object.hpp"
#include "Scene.hpp"
#include "SceneManager.hpp"
#include "lua.hpp"

#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#define CMEP_USE_FACTORY_TRAMPOLINE
#undef CMEP_USE_FACTORY_NEW_PUSH
#undef CMEP_USE_FACTORY_OLD_PUSH
#include "Scripting/API/FactoryMappings.hpp"

namespace Engine::Scripting::API::LuaFactories
{
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

	void sceneManagerFactory(lua_State* state, SceneManager* scene_manager_ptr)
	{
		// Generate SceneManager table
		lua_createtable(state, 0, static_cast<int>(scene_manager_mappings.size() + 1));

		CMEP_LUAFACTORY_PUSH_MAPPINGS(state, scene_manager_mappings)

		// Add SceneManager pointer
		lua_pushlightuserdata(state, scene_manager_ptr);
		lua_setfield(state, -2, "_ptr");
	}

	void sceneFactory(lua_State* state, Scene* scene_ptr)
	{
		// Generate Scene table
		lua_createtable(state, 0, 1);

		CMEP_LUAFACTORY_PUSH_MAPPINGS(state, scene_mappings)

		lua_pushlightuserdata(state, scene_ptr);
		lua_setfield(state, -2, "_ptr");
	}

	void objectFactory(lua_State* state, Object* object_ptr)
	{
		// Generate Object table
		lua_createtable(state, 0, static_cast<int>(object_mappings.size() + 2));

		CMEP_LUAFACTORY_PUSH_MAPPINGS(state, object_mappings)

		// Add Object pointer
		lua_pushlightuserdata(state, object_ptr);
		lua_setfield(state, -2, "_ptr");

		// Generate renderer table
		lua_pushlightuserdata(state, object_ptr->getRenderer());
		lua_setfield(state, -2, "renderer");

		lua_pushlightuserdata(state, object_ptr->getMeshBuilder());
		lua_setfield(state, -2, "meshbuilder");
	}

	void assetManagerFactory(lua_State* state, std::weak_ptr<AssetManager> asset_manager_ptr)
	{
		// Generate AssetManager table
		lua_createtable(state, 0, static_cast<int>(asset_manager_mappings.size() + 1));

		CMEP_LUAFACTORY_PUSH_MAPPINGS(state, asset_manager_mappings)

		// Add AssetManager pointer
		void* ptr_obj = lua_newuserdata(state, sizeof(std::weak_ptr<AssetManager>));
		new (ptr_obj) std::weak_ptr<AssetManager>(std::move(asset_manager_ptr));
		lua_setfield(state, -2, "_smart_ptr");
	}

	void engineFactory(lua_State* state, Engine* engine_ptr)
	{
		// Generate Engine table
		lua_createtable(state, 0, static_cast<int>(engine_mappings.size() + 1));

		CMEP_LUAFACTORY_PUSH_MAPPINGS(state, engine_mappings)

		// Add Engine pointer
		lua_pushlightuserdata(state, engine_ptr);
		lua_setfield(state, -2, "_ptr");
	}
} // namespace Engine::Scripting::API::LuaFactories
