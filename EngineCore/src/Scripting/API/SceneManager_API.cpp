#include "Scripting/API/SceneManager_API.hpp"

#include "Scripting/API/CallGenerator.hpp"
#include "Scripting/API/framework.hpp"

#include "SceneManager.hpp"
#include "lua.hpp"

#include <string>
#include <unordered_map>

namespace Engine::Scripting::API
{
	/// @cond LUA_API
	namespace
	{
		GENERATED_LAMBDA_MEMBER_CALL(SceneManager, getCameraRotation);
		GENERATED_LAMBDA_MEMBER_CALL(SceneManager, setCameraRotation);
		GENERATED_LAMBDA_MEMBER_CALL(SceneManager, getCameraTransform);
		GENERATED_LAMBDA_MEMBER_CALL(SceneManager, setCameraTransform);
		GENERATED_LAMBDA_MEMBER_CALL(SceneManager, getLightTransform);
		GENERATED_LAMBDA_MEMBER_CALL(SceneManager, setLightTransform);
		GENERATED_LAMBDA_MEMBER_CALL(SceneManager, getSceneCurrent);
		GENERATED_LAMBDA_MEMBER_CALL(SceneManager, setScene);
	} // namespace
	/// @endcond

	std::unordered_map<std::string, const lua_CFunction> scene_manager_mappings = {
		CMEP_LUAMAPPING_DEFINE(getCameraRotation),
		CMEP_LUAMAPPING_DEFINE(setCameraRotation),

		CMEP_LUAMAPPING_DEFINE(getCameraTransform),
		CMEP_LUAMAPPING_DEFINE(setCameraTransform),

		CMEP_LUAMAPPING_DEFINE(getLightTransform),
		CMEP_LUAMAPPING_DEFINE(setLightTransform),

		CMEP_LUAMAPPING_DEFINE(getSceneCurrent),
		CMEP_LUAMAPPING_DEFINE(setScene)
	};
} // namespace Engine::Scripting::API
