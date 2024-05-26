#include "Scripting/API/LuaFactories.hpp"

#include "Scripting/API/SceneManager_API.hpp"
#include "Scripting/API/Object_API.hpp"

namespace Engine::Scripting::API
{
	namespace LuaObjectFactories
	{
		void SceneManagerFactory(lua_State* state, std::weak_ptr<SceneManager> scene_manager)
		{
			// Generate scene_manager table
			lua_newtable(state);

			for(auto& mapping : sceneManager_Mappings)
			{
				lua_pushcfunction(state, mapping.second);
				lua_setfield(state, -2, mapping.first.c_str());
			}

			void* ptr_obj = lua_newuserdata(state, sizeof(std::weak_ptr<SceneManager>));
			new(ptr_obj) std::weak_ptr<SceneManager>(scene_manager);
			
			lua_setfield(state, -2, "_smart_pointer");
		}

		void ObjectFactory(lua_State* state, Object* object_ptr)
		{
			// Generate object table
			lua_newtable(state);

			// Add object pointer
			Object** ptr_obj = (Object**)lua_newuserdata(state, sizeof(Object*));
			(*ptr_obj) = object_ptr;
			lua_setfield(state, -2, "_pointer");

			// Object mappings
			for(auto& mapping : object_Mappings)
			{
				lua_pushcfunction(state, mapping.second);
				lua_setfield(state, -2, mapping.first.c_str());
			}

			// Generate renderer table
			lua_newtable(state);
			Rendering::IRenderer** ptr_renderer = (Rendering::IRenderer**)lua_newuserdata(state, sizeof(Rendering::IRenderer*));
			(*ptr_renderer) = object_ptr->renderer;
			lua_setfield(state, -2, "_pointer");
			lua_setfield(state, -2, "renderer");
		}

		std::weak_ptr<Logging::Logger> MetaLogger_Factory(lua_State* state)
		{
			lua_getglobal(state, "cmepmeta");
			lua_getfield(state, -1, "logger");
			lua_getfield(state, -1, "_smart_pointer");
			std::weak_ptr<Logging::Logger> logger = *(std::weak_ptr<Logging::Logger>*)lua_touserdata(state, -1);

			return logger;
		}
	}
}