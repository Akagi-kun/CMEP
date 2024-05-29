#include "Scripting/API/LuaFactories.hpp"

#include "Scripting/API/SceneManager_API.hpp"
#include "Scripting/API/AssetManager_API.hpp"
#include "Scripting/API/Object_API.hpp"

namespace Engine::Scripting::API
{
	namespace LuaObjectFactories
	{
		void SceneManagerFactory(lua_State *state, std::weak_ptr<SceneManager> scene_manager_ptr)
		{
			// Generate SceneManager table
			lua_newtable(state);

			// Add SceneManager mappings
			for (auto &mapping : sceneManager_Mappings)
			{
				lua_pushcfunction(state, mapping.second);
				lua_setfield(state, -2, mapping.first.c_str());
			}

			// Add SceneManager pointer
			void *ptr_obj = lua_newuserdata(state, sizeof(std::weak_ptr<SceneManager>));
			new (ptr_obj) std::weak_ptr<SceneManager>(scene_manager_ptr);

			lua_setfield(state, -2, "_smart_pointer");
		}

		void ObjectFactory(lua_State *state, Object *object_ptr)
		{
			// Generate Object table
			lua_newtable(state);

			// Object mappings
			for (auto &mapping : object_Mappings)
			{
				lua_pushcfunction(state, mapping.second);
				lua_setfield(state, -2, mapping.first.c_str());
			}

			// Add Object pointer
			Object **ptr_obj = (Object **)lua_newuserdata(state, sizeof(Object *));
			(*ptr_obj) = object_ptr;
			lua_setfield(state, -2, "_pointer");

			// Generate renderer table
			lua_newtable(state);
			Rendering::IRenderer **ptr_renderer = (Rendering::IRenderer **)lua_newuserdata(state, sizeof(Rendering::IRenderer *));
			(*ptr_renderer) = object_ptr->renderer;
			lua_setfield(state, -2, "_pointer");
			lua_setfield(state, -2, "renderer");
		}

		void AssetManagerFactory(lua_State *state, std::weak_ptr<AssetManager> asset_manager_ptr)
		{
			// Generate AssetManager table
			lua_newtable(state);

			// AssetManager mappings
			for (auto &mapping : assetManager_Mappings)
			{
				lua_pushcfunction(state, mapping.second);
				lua_setfield(state, -2, mapping.first.c_str());
			}

			// Add AssetManager pointer
			void *ptr_obj = lua_newuserdata(state, sizeof(std::weak_ptr<AssetManager>));
			new (ptr_obj) std::weak_ptr<AssetManager>(asset_manager_ptr);
			lua_setfield(state, -2, "_smart_pointer");
		}

		std::weak_ptr<Logging::Logger> MetaLogger_Factory(lua_State *state)
		{
			lua_getglobal(state, "cmepmeta");
			lua_getfield(state, -1, "logger");
			lua_getfield(state, -1, "_smart_pointer");
			std::weak_ptr<Logging::Logger> logger = *(std::weak_ptr<Logging::Logger> *)lua_touserdata(state, -1);

			return logger;
		}
	}
}