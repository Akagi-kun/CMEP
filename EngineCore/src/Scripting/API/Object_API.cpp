#include "Scripting/API/Object_API.hpp"

#include "Scripting/API/framework.hpp"

#include "SceneObject.hpp"
#include "lua.hpp"

#include <string>
#include <unordered_map>

namespace Engine::Scripting::API
{
	/// @cond LUA_API
	namespace
	{
		int addChild(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 2)
			CMEP_LUAGET_PTR(state, SceneObject)

			lua_getfield(state, 2, "_ptr");
			auto* ptr_child = static_cast<SceneObject*>(lua_touserdata(state, -1));

			self->addChild(ptr_child);

			return 0;
		}

		int getRotation(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 1)
			CMEP_LUAGET_PTR(state, SceneObject)

			glm::vec3 rotation = self->getRotation();

			lua_pushnumber(state, static_cast<double>(rotation.x));
			lua_pushnumber(state, static_cast<double>(rotation.y));
			lua_pushnumber(state, static_cast<double>(rotation.z));

			return 3;
		}

		int setRotation(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 4)
			CMEP_LUAGET_PTR(state, SceneObject)

			glm::vec3 rotation;
			rotation.x = static_cast<float>(lua_tonumber(state, 2));
			rotation.y = static_cast<float>(lua_tonumber(state, 3));
			rotation.z = static_cast<float>(lua_tonumber(state, 4));

			self->setRotation(rotation);

			return 0;
		}

		int getSize(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 1)
			CMEP_LUAGET_PTR(state, SceneObject)

			glm::vec3 size = self->getSize();

			lua_pushnumber(state, static_cast<double>(size.x));
			lua_pushnumber(state, static_cast<double>(size.y));
			lua_pushnumber(state, static_cast<double>(size.z));

			return 3;
		}

		int setSize(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 4)
			CMEP_LUAGET_PTR(state, SceneObject)

			glm::vec3 size;
			size.x = static_cast<float>(lua_tonumber(state, 2));
			size.y = static_cast<float>(lua_tonumber(state, 3));
			size.z = static_cast<float>(lua_tonumber(state, 4));

			self->setSize(size);

			return 0;
		}

		int getPosition(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 1)
			CMEP_LUAGET_PTR(state, SceneObject)

			auto rotation = self->getPosition();

			lua_pushnumber(state, static_cast<double>(rotation.x));
			lua_pushnumber(state, static_cast<double>(rotation.y));
			lua_pushnumber(state, static_cast<double>(rotation.z));

			return 3;
		}

		int setPosition(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 4)
			CMEP_LUAGET_PTR(state, SceneObject)

			glm::vec3 position;
			position.x = static_cast<float>(lua_tonumber(state, 2));
			position.y = static_cast<float>(lua_tonumber(state, 3));
			position.z = static_cast<float>(lua_tonumber(state, 4));

			self->setPosition(position);

			return 0;
		}

	} // namespace
	/// @endcond

	std::unordered_map<std::string, const lua_CFunction> object_mappings = {
		CMEP_LUAMAPPING_DEFINE(addChild),
		CMEP_LUAMAPPING_DEFINE(getSize),
		CMEP_LUAMAPPING_DEFINE(setSize),
		CMEP_LUAMAPPING_DEFINE(getRotation),
		CMEP_LUAMAPPING_DEFINE(setRotation),
		CMEP_LUAMAPPING_DEFINE(getPosition),
		CMEP_LUAMAPPING_DEFINE(setPosition)
	};
} // namespace Engine::Scripting::API
