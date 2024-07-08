#include "Scripting/API/SceneManager_API.hpp"

#include "Scripting/API/LuaFactories.hpp"
#include "Scripting/API/framework.hpp"

#include "SceneManager.hpp"
#include "lua.h"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_LUA_MAPPED
#include "Logging/LoggingPrefix.hpp"

namespace Engine::Scripting::API
{
	namespace Functions_SceneManager
	{

#pragma region SceneManager

		static int GetCameraHVRotation(lua_State* state)
		{
			CMEP_CHECK_FN_ARGC(state, 1);

			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *static_cast<std::weak_ptr<SceneManager>*>(
				lua_touserdata(state, -1)
			);

			if (auto locked_scene_manager = scene_manager.lock())
			{
				glm::vec2 hvrot = locked_scene_manager->GetCameraHVRotation();

				lua_pushnumber(state, static_cast<double>(hvrot.x));
				lua_pushnumber(state, static_cast<double>(hvrot.y));

				return 2;
			}

			return luaL_error(state, "Could not lock scene manager!");
		}

		static int SetCameraHVRotation(lua_State* state)
		{
			CMEP_CHECK_FN_ARGC(state, 3);

			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *static_cast<std::weak_ptr<SceneManager>*>(
				lua_touserdata(state, -1)
			);

			if (auto locked_scene_manager = scene_manager.lock())
			{
				double h_rot = lua_tonumber(state, 2);
				double v_rot = lua_tonumber(state, 3);

				locked_scene_manager->SetCameraHVRotation(glm::vec2(h_rot, v_rot));

				return 0;
			}

			return luaL_error(state, "Could not lock scene manager!");
		}

		static int GetCameraTransform(lua_State* state)
		{
			CMEP_CHECK_FN_ARGC(state, 1);

			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *static_cast<std::weak_ptr<SceneManager>*>(
				lua_touserdata(state, -1)
			);

			if (auto locked_scene_manager = scene_manager.lock())
			{
				glm::vec3 transform = locked_scene_manager->GetCameraTransform();

				lua_pushnumber(state, static_cast<double>(transform.x));
				lua_pushnumber(state, static_cast<double>(transform.y));
				lua_pushnumber(state, static_cast<double>(transform.z));

				return 3;
			}

			return luaL_error(state, "Could not lock scene manager!");
		}

		static int SetCameraTransform(lua_State* state)
		{
			CMEP_CHECK_FN_ARGC(state, 4);

			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *static_cast<std::weak_ptr<SceneManager>*>(
				lua_touserdata(state, -1)
			);

			if (auto locked_scene_manager = scene_manager.lock())
			{
				double x_pos = lua_tonumber(state, 2);
				double y_pos = lua_tonumber(state, 3);
				double z_pos = lua_tonumber(state, 4);

				locked_scene_manager->SetCameraTransform(glm::vec3(x_pos, y_pos, z_pos));

				return 0;
			}

			return luaL_error(state, "Could not lock scene manager!");
		}

		static int GetLightTransform(lua_State* state)
		{
			CMEP_CHECK_FN_ARGC(state, 1);

			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *static_cast<std::weak_ptr<SceneManager>*>(
				lua_touserdata(state, -1)
			);

			if (auto locked_scene_manager = scene_manager.lock())
			{
				glm::vec3 transform = locked_scene_manager->GetLightTransform();

				lua_pushnumber(state, static_cast<double>(transform.x));
				lua_pushnumber(state, static_cast<double>(transform.y));
				lua_pushnumber(state, static_cast<double>(transform.z));

				return 3;
			}

			return luaL_error(state, "Could not lock scene manager!");
		}

		static int SetLightTransform(lua_State* state)
		{
			CMEP_CHECK_FN_ARGC(state, 4);

			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *static_cast<std::weak_ptr<SceneManager>*>(
				lua_touserdata(state, -1)
			);

			if (auto locked_scene_manager = scene_manager.lock())
			{
				double x_pos = lua_tonumber(state, 2);
				double y_pos = lua_tonumber(state, 3);
				double z_pos = lua_tonumber(state, 4);

				locked_scene_manager->SetLightTransform(glm::vec3(x_pos, y_pos, z_pos));

				return 0;
			}

			return luaL_error(state, "Could not lock scene manager!");
		}

		static int AddObject(lua_State* state)
		{
			CMEP_CHECK_FN_ARGC(state, 3);

			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *static_cast<std::weak_ptr<SceneManager>*>(
				lua_touserdata(state, -1)
			);

			std::string name = lua_tostring(state, 2);

			lua_getfield(state, 3, "_pointer");
			auto* ptr_obj = static_cast<Object*>(lua_touserdata(state, -1));

			if (auto locked_scene_manager = scene_manager.lock())
			{
				locked_scene_manager->GetSceneCurrent()->AddObject(name, ptr_obj);

				return 0;
			}

			return luaL_error(state, "Could not lock scene manager!");
		}

		static int FindObject(lua_State* state)
		{
			CMEP_CHECK_FN_ARGC(state, 2);

			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *static_cast<std::weak_ptr<SceneManager>*>(
				lua_touserdata(state, -1)
			);

			std::string obj_name = lua_tostring(state, 2);

			if (auto locked_scene_manager = scene_manager.lock())
			{
				Object* obj = locked_scene_manager->FindObject(obj_name);

				if (obj != nullptr)
				{
					API::LuaObjectFactories::ObjectFactory(state, obj);

					return 1;
				}

				return luaL_error(state, "Object %s requested but returned nullptr!", obj_name.c_str());
			}

			return luaL_error(state, "Could not lock scene manager!");
		}

		static int RemoveObject(lua_State* state)
		{
			CMEP_CHECK_FN_ARGC(state, 2);

			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *static_cast<std::weak_ptr<SceneManager>*>(
				lua_touserdata(state, -1)
			);

			std::string name = lua_tostring(state, 2);

			if (auto locked_scene_manager = scene_manager.lock())
			{
				locked_scene_manager->RemoveObject(name);

				return 0;
			}

			return luaL_error(state, "Could not lock scene manager!");
		}

		static int AddTemplatedObject(lua_State* state)
		{
			CMEP_CHECK_FN_ARGC(state, 3);

			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *static_cast<std::weak_ptr<SceneManager>*>(
				lua_touserdata(state, -1)
			);

			std::string name		  = lua_tostring(state, 2);
			std::string template_name = lua_tostring(state, 3);

			if (auto locked_scene_manager = scene_manager.lock())
			{
				locked_scene_manager->AddTemplatedObject(name, template_name);

				return 0;
			}

			return luaL_error(state, "Could not lock scene manager!");
		}
#pragma endregion

	} // namespace Functions_SceneManager

	const std::unordered_map<std::string, lua_CFunction> scene_manager_mappings = {
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, GetCameraHVRotation),
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, SetCameraHVRotation),
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, GetCameraTransform),
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, SetCameraTransform),
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, GetLightTransform),
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, SetLightTransform),
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, AddObject),
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, FindObject),
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, RemoveObject),
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, AddTemplatedObject)
	};
} // namespace Engine::Scripting::API
