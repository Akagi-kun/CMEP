#include "Scripting/API/Object_API.hpp"

#include "Scripting/API/CallGenerator.hpp"
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
		GENERATED_LAMBDA_MEMBER_CALL(SceneObject, addChild)
		GENERATED_LAMBDA_MEMBER_CALL(SceneObject, getRotation)
		GENERATED_LAMBDA_MEMBER_CALL(SceneObject, setRotation)
		GENERATED_LAMBDA_MEMBER_CALL(SceneObject, getSize)
		GENERATED_LAMBDA_MEMBER_CALL(SceneObject, setSize)
		GENERATED_LAMBDA_MEMBER_CALL(SceneObject, getPosition)
		GENERATED_LAMBDA_MEMBER_CALL(SceneObject, setPosition)

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
