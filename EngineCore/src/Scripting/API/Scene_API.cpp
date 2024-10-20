#include "Scripting/API/Scene_API.hpp"

#include "Scripting/API/LuaFactories.hpp"
#include "Scripting/API/framework.hpp"

#include "Scene.hpp"
#include "SceneObject.hpp"

#include <string>
#include <unordered_map>

namespace Engine::Scripting::API
{
	/// @cond LUA_API
	namespace
	{
		int addObject(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 3)

			lua_getfield(state, 1, "_ptr");
			auto* scene = static_cast<Scene*>(lua_touserdata(state, -1));

			std::string name = lua_tostring(state, 2);

			lua_getfield(state, 3, "_ptr");
			auto* ptr_obj = static_cast<SceneObject*>(lua_touserdata(state, -1));

			scene->addObject(name, ptr_obj);

			return 0;
		}

		int findObject(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 2)

			lua_getfield(state, 1, "_ptr");
			auto* scene = static_cast<Scene*>(lua_touserdata(state, -1));

			std::string obj_name = lua_tostring(state, 2);

			SceneObject* obj = scene->findObject(obj_name);

			if (obj == nullptr)
			{
				return luaL_error(
					state,
					"Object '%s' requested but returned nullptr!",
					obj_name.c_str()
				);
			}

			API::LuaFactories::objectFactory(state, obj);

			return 1;
		}

		int removeObject(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 2)

			lua_getfield(state, 1, "_ptr");
			auto* scene = static_cast<Scene*>(lua_touserdata(state, -1));

			std::string name = lua_tostring(state, 2);

			scene->removeObject(name);

			return 0;
		}

		int addTemplatedObject(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 3)

			lua_getfield(state, 1, "_ptr");
			auto* scene = static_cast<Scene*>(lua_touserdata(state, -1));

			std::string name		  = lua_tostring(state, 2);
			std::string template_name = lua_tostring(state, 3);

			scene->addTemplatedObject(name, template_name);
			SceneObject* obj = scene->findObject(name);

			if (obj == nullptr)
			{
				return luaL_error(
					state,
					"Object could not be added (FindObject on '%s' returned nullptr!)",
					name.c_str()
				);
			}

			API::LuaFactories::objectFactory(state, obj);

			return 1;
		}
	} // namespace
	/// @endcond

	std::unordered_map<std::string, const lua_CFunction> scene_mappings = {
		CMEP_LUAMAPPING_DEFINE(addObject),
		CMEP_LUAMAPPING_DEFINE(findObject),
		CMEP_LUAMAPPING_DEFINE(removeObject),
		CMEP_LUAMAPPING_DEFINE(addTemplatedObject)
	};
} // namespace Engine::Scripting::API
