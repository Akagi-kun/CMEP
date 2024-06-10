#include "Scripting/API/Object_API.hpp"

#include "Object.hpp"

namespace Engine::Scripting::API
{
	namespace Functions_Object
	{
		int AddChild(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");

			Object* ptr_obj = *(Object**)lua_touserdata(state, -1);

			lua_getfield(state, 2, "_pointer");

			Object* ptr_child = *(Object**)lua_touserdata(state, -1);

			ptr_obj->AddChild(ptr_child);

			return 0;
		}

		int GetRotation(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");
			Object* ptr_obj = *(Object**)lua_touserdata(state, -1);

			glm::vec3 rotation = ptr_obj->Rotation();

			lua_pushnumber(state, rotation.x);
			lua_pushnumber(state, rotation.y);
			lua_pushnumber(state, rotation.z);

			return 3;
		}

		int Rotate(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");

			Object* ptr_obj = *(Object**)lua_touserdata(state, -1);

			glm::vec3 rotation = glm::vec3(0);
			rotation.x = static_cast<float>(lua_tonumber(state, 2));
			rotation.y = static_cast<float>(lua_tonumber(state, 3));
			rotation.z = static_cast<float>(lua_tonumber(state, 4));

			ptr_obj->Rotate(rotation);

			return 0;
		}

		int GetSize(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");
			Object* ptr_obj = *(Object**)lua_touserdata(state, -1);

			glm::vec3 size = ptr_obj->Size();

			lua_pushnumber(state, size.x);
			lua_pushnumber(state, size.y);
			lua_pushnumber(state, size.z);

			return 3;
		}

		int Scale(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");

			Object* ptr_obj = *(Object**)lua_touserdata(state, -1);

			glm::vec3 size = glm::vec3(0);
			size.x = static_cast<float>(lua_tonumber(state, 2));
			size.y = static_cast<float>(lua_tonumber(state, 3));
			size.z = static_cast<float>(lua_tonumber(state, 4));

			ptr_obj->Scale(size);

			return 0;
		}

		int GetPosition(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");
			Object* ptr_obj = *(Object**)lua_touserdata(state, -1);

			glm::vec3 rotation = ptr_obj->Position();

			lua_pushnumber(state, rotation.x);
			lua_pushnumber(state, rotation.y);
			lua_pushnumber(state, rotation.z);

			return 3;
		}

		int Translate(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");

			Object* ptr_obj = *(Object**)lua_touserdata(state, -1);

			glm::vec3 position = glm::vec3(0);
			position.x = static_cast<float>(lua_tonumber(state, 2));
			position.y = static_cast<float>(lua_tonumber(state, 3));
			position.z = static_cast<float>(lua_tonumber(state, 4));

			ptr_obj->Translate(position);

			return 0;
		}
	} // namespace Functions_Object

	std::unordered_map<std::string, lua_CFunction> object_mappings = {
		CMEP_LUAMAPPING_DEFINE(Functions_Object, AddChild),
		CMEP_LUAMAPPING_DEFINE(Functions_Object, GetSize),
		CMEP_LUAMAPPING_DEFINE(Functions_Object, Scale),
		CMEP_LUAMAPPING_DEFINE(Functions_Object, GetRotation),
		CMEP_LUAMAPPING_DEFINE(Functions_Object, Rotate),
		CMEP_LUAMAPPING_DEFINE(Functions_Object, GetPosition),
		CMEP_LUAMAPPING_DEFINE(Functions_Object, Translate)
	};
} // namespace Engine::Scripting::API