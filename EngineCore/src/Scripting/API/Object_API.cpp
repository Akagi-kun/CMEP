#include "Scripting/API/Object_API.hpp"

#include "Scripting/API/framework.hpp"

#include "Object.hpp"
#include "lua.h"

namespace Engine::Scripting::API
{
	namespace Functions_Object
	{
		using glm::vec3;

		static int AddChild(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");
			auto* ptr_obj = static_cast<Object*>(lua_touserdata(state, -1));

			lua_getfield(state, 2, "_pointer");
			auto* ptr_child = static_cast<Object*>(lua_touserdata(state, -1));

			ptr_obj->AddChild(ptr_child);

			return 0;
		}

		static int GetRotation(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");
			auto* ptr_obj = static_cast<Object*>(lua_touserdata(state, -1));

			glm::vec3 rotation = ptr_obj->Rotation();

			lua_pushnumber(state, static_cast<double>(rotation.x));
			lua_pushnumber(state, static_cast<double>(rotation.y));
			lua_pushnumber(state, static_cast<double>(rotation.z));

			return 3;
		}

		static int Rotate(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");
			auto* ptr_obj = static_cast<Object*>(lua_touserdata(state, -1));

			glm::vec3 rotation;
			rotation.x = static_cast<float>(lua_tonumber(state, 2));
			rotation.y = static_cast<float>(lua_tonumber(state, 3));
			rotation.z = static_cast<float>(lua_tonumber(state, 4));

			ptr_obj->Rotate(rotation);

			return 0;
		}

		static int GetSize(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");
			auto* ptr_obj = static_cast<Object*>(lua_touserdata(state, -1));

			glm::vec3 size = ptr_obj->Size();

			lua_pushnumber(state, static_cast<double>(size.x));
			lua_pushnumber(state, static_cast<double>(size.y));
			lua_pushnumber(state, static_cast<double>(size.z));

			return 3;
		}

		static int Scale(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");
			auto* ptr_obj = static_cast<Object*>(lua_touserdata(state, -1));

			glm::vec3 size;
			size.x = static_cast<float>(lua_tonumber(state, 2));
			size.y = static_cast<float>(lua_tonumber(state, 3));
			size.z = static_cast<float>(lua_tonumber(state, 4));

			ptr_obj->Scale(size);

			return 0;
		}

		static int GetPosition(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");
			auto* ptr_obj = static_cast<Object*>(lua_touserdata(state, -1));

			auto rotation = ptr_obj->Position();

			lua_pushnumber(state, static_cast<double>(rotation.x));
			lua_pushnumber(state, static_cast<double>(rotation.y));
			lua_pushnumber(state, static_cast<double>(rotation.z));

			return 3;
		}

		static int Translate(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");
			auto* ptr_obj = static_cast<Object*>(lua_touserdata(state, -1));

			glm::vec3 position;
			position.x = static_cast<float>(lua_tonumber(state, 2));
			position.y = static_cast<float>(lua_tonumber(state, 3));
			position.z = static_cast<float>(lua_tonumber(state, 4));

			ptr_obj->Translate(position);

			return 0;
		}
	} // namespace Functions_Object

	const std::unordered_map<std::string, lua_CFunction> object_mappings = {
		CMEP_LUAMAPPING_DEFINE(Functions_Object, AddChild),
		CMEP_LUAMAPPING_DEFINE(Functions_Object, GetSize),
		CMEP_LUAMAPPING_DEFINE(Functions_Object, Scale),
		CMEP_LUAMAPPING_DEFINE(Functions_Object, GetRotation),
		CMEP_LUAMAPPING_DEFINE(Functions_Object, Rotate),
		CMEP_LUAMAPPING_DEFINE(Functions_Object, GetPosition),
		CMEP_LUAMAPPING_DEFINE(Functions_Object, Translate)
	};
} // namespace Engine::Scripting::API
