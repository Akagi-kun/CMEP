#include "Scripting/API/SceneManager_API.hpp"

#include "Scripting/API/LuaFactories.hpp"
#include "Scripting/API/framework.hpp"

#include "SceneManager.hpp"
#include "lua.hpp"

#include <string>
#include <unordered_map>

namespace Engine::Scripting::API
{
	namespace
	{
		int getCameraRotation(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 1)
			CMEP_LUAGET_PTR(state, SceneManager)

			glm::vec2 hvrot = self->getCameraRotation();

			lua_pushnumber(state, static_cast<double>(hvrot.x));
			lua_pushnumber(state, static_cast<double>(hvrot.y));

			return 2;
		}

		int setCameraRotation(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 3)
			CMEP_LUAGET_PTR(state, SceneManager)

			double h_rot = lua_tonumber(state, 2);
			double v_rot = lua_tonumber(state, 3);

			self->setCameraRotation(glm::vec2(h_rot, v_rot));

			return 0;
		}

		int getCameraTransform(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 1)
			CMEP_LUAGET_PTR(state, SceneManager)

			glm::vec3 transform = self->getCameraTransform();

			lua_pushnumber(state, static_cast<double>(transform.x));
			lua_pushnumber(state, static_cast<double>(transform.y));
			lua_pushnumber(state, static_cast<double>(transform.z));

			return 3;
		}

		int setCameraTransform(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 4)
			CMEP_LUAGET_PTR(state, SceneManager)

			double x_pos = lua_tonumber(state, 2);
			double y_pos = lua_tonumber(state, 3);
			double z_pos = lua_tonumber(state, 4);

			self->setCameraTransform(glm::vec3(x_pos, y_pos, z_pos));

			return 0;
		}

		int getLightTransform(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 1)
			CMEP_LUAGET_PTR(state, SceneManager)

			glm::vec3 transform = self->getLightTransform();

			lua_pushnumber(state, static_cast<double>(transform.x));
			lua_pushnumber(state, static_cast<double>(transform.y));
			lua_pushnumber(state, static_cast<double>(transform.z));

			return 3;
		}

		int setLightTransform(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 4)
			CMEP_LUAGET_PTR(state, SceneManager)

			double x_pos = lua_tonumber(state, 2);
			double y_pos = lua_tonumber(state, 3);
			double z_pos = lua_tonumber(state, 4);

			self->setLightTransform(glm::vec3(x_pos, y_pos, z_pos));

			return 0;
		}

		int getSceneCurrent(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 1)
			CMEP_LUAGET_PTR(state, SceneManager)

			auto& scene = self->getSceneCurrent();

			LuaFactories::sceneFactory(state, scene.get());

			return 1;
		}

	} // namespace

	std::unordered_map<std::string, const lua_CFunction> scene_manager_mappings = {
		CMEP_LUAMAPPING_DEFINE(getCameraRotation),
		CMEP_LUAMAPPING_DEFINE(setCameraRotation),
		CMEP_LUAMAPPING_DEFINE(getCameraTransform),
		CMEP_LUAMAPPING_DEFINE(setCameraTransform),
		CMEP_LUAMAPPING_DEFINE(getLightTransform),
		CMEP_LUAMAPPING_DEFINE(setLightTransform),
		CMEP_LUAMAPPING_DEFINE(getSceneCurrent)
	};
} // namespace Engine::Scripting::API
