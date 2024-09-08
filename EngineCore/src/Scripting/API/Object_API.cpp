#include "Scripting/API/Object_API.hpp"

#include "Scripting/API/framework.hpp"

#include "Object.hpp"
#include "lua.hpp"

#include <string>
#include <unordered_map>

namespace Engine::Scripting::API
{
	namespace Functions_Object
	{
		static int AddChild(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 2)
			CMEP_LUAGET_PTR(state, Object)

			lua_getfield(state, 2, "_ptr");
			auto* ptr_child = static_cast<Object*>(lua_touserdata(state, -1));

			self->addChild(ptr_child);

			return 0;
		}

		static int GetRotation(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 1)
			CMEP_LUAGET_PTR(state, Object)

			glm::vec3 rotation = self->getRotation();

			lua_pushnumber(state, static_cast<double>(rotation.x));
			lua_pushnumber(state, static_cast<double>(rotation.y));
			lua_pushnumber(state, static_cast<double>(rotation.z));

			return 3;
		}

		static int SetRotation(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 4)
			CMEP_LUAGET_PTR(state, Object)

			glm::vec3 rotation;
			rotation.x = static_cast<float>(lua_tonumber(state, 2));
			rotation.y = static_cast<float>(lua_tonumber(state, 3));
			rotation.z = static_cast<float>(lua_tonumber(state, 4));

			self->setRotation(rotation);

			return 0;
		}

		static int GetSize(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 1)
			CMEP_LUAGET_PTR(state, Object)

			glm::vec3 size = self->getSize();

			lua_pushnumber(state, static_cast<double>(size.x));
			lua_pushnumber(state, static_cast<double>(size.y));
			lua_pushnumber(state, static_cast<double>(size.z));

			return 3;
		}

		static int SetSize(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 4)
			CMEP_LUAGET_PTR(state, Object)

			glm::vec3 size;
			size.x = static_cast<float>(lua_tonumber(state, 2));
			size.y = static_cast<float>(lua_tonumber(state, 3));
			size.z = static_cast<float>(lua_tonumber(state, 4));

			self->setSize(size);

			return 0;
		}

		static int GetPosition(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 1)
			CMEP_LUAGET_PTR(state, Object)

			auto rotation = self->getPosition();

			lua_pushnumber(state, static_cast<double>(rotation.x));
			lua_pushnumber(state, static_cast<double>(rotation.y));
			lua_pushnumber(state, static_cast<double>(rotation.z));

			return 3;
		}

		static int SetPosition(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 4)
			CMEP_LUAGET_PTR(state, Object)

			glm::vec3 position;
			position.x = static_cast<float>(lua_tonumber(state, 2));
			position.y = static_cast<float>(lua_tonumber(state, 3));
			position.z = static_cast<float>(lua_tonumber(state, 4));

			self->setPosition(position);

			return 0;
		}
	} // namespace Functions_Object

	std::unordered_map<std::string, const lua_CFunction> object_mappings = {
		CMEP_LUAMAPPING_DEFINE(Functions_Object, AddChild),
		CMEP_LUAMAPPING_DEFINE(Functions_Object, GetSize),
		CMEP_LUAMAPPING_DEFINE(Functions_Object, SetSize),
		CMEP_LUAMAPPING_DEFINE(Functions_Object, GetRotation),
		CMEP_LUAMAPPING_DEFINE(Functions_Object, SetRotation),
		CMEP_LUAMAPPING_DEFINE(Functions_Object, GetPosition),
		CMEP_LUAMAPPING_DEFINE(Functions_Object, SetPosition)
	};
} // namespace Engine::Scripting::API
