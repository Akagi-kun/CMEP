#include "Scripting/API/Engine_API.hpp"

#include "Scripting/API/LuaFactories.hpp"
#include "Scripting/API/framework.hpp"

#include "Engine.hpp"
#include "SceneManager.hpp"
#include "lua.hpp"

#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

namespace Engine::Scripting::API
{
	namespace
	{
		int getAssetManager(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 1)
			CMEP_LUAGET_PTR(state, Engine)

			std::weak_ptr<AssetManager> asset_manager = self->getAssetManager();

			if (asset_manager.expired())
			{
				return luaL_error(state, "AssetManager is expired");
			}

			API::LuaFactories::assetManagerFactory(state, asset_manager);

			return 1;
		}

		int getSceneManager(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 1)
			CMEP_LUAGET_PTR(state, Engine)

			std::weak_ptr<SceneManager> scene_manager = self->getSceneManager();

			auto locked_scene_manager = scene_manager.lock();
			assert(locked_scene_manager);

			API::LuaFactories::sceneManagerFactory(state, locked_scene_manager.get());

			return 1;
		}

		int setFramerateTarget(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 2)
			CMEP_LUAGET_PTR(state, Engine)

			auto framerate_target = static_cast<uint_fast16_t>(lua_tointeger(state, 2));

			self->setFramerateTarget(framerate_target);

			return 0;
		}

		int stop(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 1)
			CMEP_LUAGET_PTR(state, Engine)

			self->stop();

			return 0;
		}

	} // namespace

	std::unordered_map<std::string, const lua_CFunction> engine_mappings = {
		CMEP_LUAMAPPING_DEFINE(getAssetManager),
		CMEP_LUAMAPPING_DEFINE(getSceneManager),
		CMEP_LUAMAPPING_DEFINE(setFramerateTarget),
		CMEP_LUAMAPPING_DEFINE(stop),
	};
} // namespace Engine::Scripting::API
