#include "Scripting/API/SceneManager_API.hpp"

#include "Scripting/API/LuaFactories.hpp"
#include "Scripting/API/framework.hpp"

#include "SceneManager.hpp"
#include "lua.h"

namespace Engine::Scripting::API
{
	namespace Functions_SceneManager
	{
		static int GetCameraHVRotation(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 1)
			CMEP_LUAGET_PTR(state, SceneManager)

			glm::vec2 hvrot = self->GetCameraHVRotation();

			lua_pushnumber(state, static_cast<double>(hvrot.x));
			lua_pushnumber(state, static_cast<double>(hvrot.y));

			return 2;
		}

		static int SetCameraHVRotation(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 3)
			CMEP_LUAGET_PTR(state, SceneManager)

			double h_rot = lua_tonumber(state, 2);
			double v_rot = lua_tonumber(state, 3);

			self->SetCameraHVRotation(glm::vec2(h_rot, v_rot));

			return 0;
		}

		static int GetCameraTransform(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 1)
			CMEP_LUAGET_PTR(state, SceneManager)

			glm::vec3 transform = self->GetCameraTransform();

			lua_pushnumber(state, static_cast<double>(transform.x));
			lua_pushnumber(state, static_cast<double>(transform.y));
			lua_pushnumber(state, static_cast<double>(transform.z));

			return 3;
		}

		static int SetCameraTransform(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 4)
			CMEP_LUAGET_PTR(state, SceneManager)

			double x_pos = lua_tonumber(state, 2);
			double y_pos = lua_tonumber(state, 3);
			double z_pos = lua_tonumber(state, 4);

			self->SetCameraTransform(glm::vec3(x_pos, y_pos, z_pos));

			return 0;
		}

		static int GetLightTransform(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 1)
			CMEP_LUAGET_PTR(state, SceneManager)

			glm::vec3 transform = self->GetLightTransform();

			lua_pushnumber(state, static_cast<double>(transform.x));
			lua_pushnumber(state, static_cast<double>(transform.y));
			lua_pushnumber(state, static_cast<double>(transform.z));

			return 3;
		}

		static int SetLightTransform(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 4)
			CMEP_LUAGET_PTR(state, SceneManager)

			double x_pos = lua_tonumber(state, 2);
			double y_pos = lua_tonumber(state, 3);
			double z_pos = lua_tonumber(state, 4);

			self->SetLightTransform(glm::vec3(x_pos, y_pos, z_pos));

			return 0;
		}

		static int GetSceneCurrent(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 1)
			CMEP_LUAGET_PTR(state, SceneManager)

			auto& scene = self->GetSceneCurrent();

			LuaFactories::SceneFactory(state, scene.get());

			return 1;
		}

	} // namespace Functions_SceneManager

	std::unordered_map<std::string, const lua_CFunction> scene_manager_mappings = {
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, GetCameraHVRotation),
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, SetCameraHVRotation),
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, GetCameraTransform),
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, SetCameraTransform),
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, GetLightTransform),
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, SetLightTransform),
		CMEP_LUAMAPPING_DEFINE(Functions_SceneManager, GetSceneCurrent)
	};
} // namespace Engine::Scripting::API
