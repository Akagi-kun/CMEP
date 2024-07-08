#include "Scripting/API/Scene_API.hpp"

#include "Scripting/API/LuaFactories.hpp"
#include "Scripting/API/framework.hpp"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_LUA_MAPPED
#include "Logging/LoggingPrefix.hpp"

namespace Engine::Scripting::API
{
	namespace Functions_Scene
	{
		static int AddObject(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 3);

			lua_getfield(state, 1, "_ptr");
			auto* scene = static_cast<Scene*>(lua_touserdata(state, -1));

			std::string name = lua_tostring(state, 2);

			lua_getfield(state, 3, "_ptr");
			auto* ptr_obj = static_cast<Object*>(lua_touserdata(state, -1));

			scene->AddObject(name, ptr_obj);

			return 0;
		}

		static int FindObject(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 2);

			lua_getfield(state, 1, "_ptr");
			auto* scene = static_cast<Scene*>(lua_touserdata(state, -1));

			std::string obj_name = lua_tostring(state, 2);

			Object* obj = scene->FindObject(obj_name);

			if (obj != nullptr)
			{
				API::LuaFactories::ObjectFactory(state, obj);

				return 1;
			}

			return luaL_error(state, "Object %s requested but returned nullptr!", obj_name.c_str());
		}

		static int RemoveObject(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 2);

			lua_getfield(state, 1, "_ptr");
			auto* scene = static_cast<Scene*>(lua_touserdata(state, -1));

			std::string name = lua_tostring(state, 2);

			scene->RemoveObject(name);

			return 0;
		}

		static int AddTemplatedObject(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 3);

			lua_getfield(state, 1, "_ptr");
			auto* scene = static_cast<Scene*>(lua_touserdata(state, -1));

			std::string name		  = lua_tostring(state, 2);
			std::string template_name = lua_tostring(state, 3);

			scene->AddTemplatedObject(name, template_name);

			return 0;
		}
	} // namespace Functions_Scene

	const std::unordered_map<std::string, lua_CFunction> scene_mappings = {
		CMEP_LUAMAPPING_DEFINE(Functions_Scene, AddObject),
		CMEP_LUAMAPPING_DEFINE(Functions_Scene, FindObject),
		CMEP_LUAMAPPING_DEFINE(Functions_Scene, RemoveObject),
		CMEP_LUAMAPPING_DEFINE(Functions_Scene, AddTemplatedObject)
	};
} // namespace Engine::Scripting::API
