#include "Scripting/API/Scene_API.hpp"

#include "Scripting/API/CallGenerator.hpp"
#include "Scripting/API/LuaFactories.hpp"
#include "Scripting/API/framework.hpp"
#include "Scripting/LuaValue.hpp"

#include "Exception.hpp"
#include "Scene.hpp"
#include "SceneObject.hpp"

#include <string>
#include <unordered_map>

namespace Engine::Scripting::API
{
	/// @cond LUA_API
	namespace
	{
		GENERATED_LAMBDA_MEMBER_CALL(Scene, addObject)
		GENERATED_LAMBDA_MEMBER_CALL(Scene, findObject)
		GENERATED_LAMBDA_MEMBER_CALL(Scene, removeObject)

		int addTemplatedObject(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 3)

			auto* scene = getObjectAsPointer<Scene>(state, 1);

			std::string name		  = LuaValue(state, 2);
			std::string template_name = LuaValue(state, 3);

			scene->addTemplatedObject(name, template_name);
			SceneObject* obj = scene->findObject(name);

			EXCEPTION_ASSERT(obj != nullptr, "Templated object could not be added");

			API::LuaFactories::templatedFactory<SceneObject>(state, obj);

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
