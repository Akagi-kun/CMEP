#include "Scripting/API/Engine_API.hpp"

#include "Scripting/API/LuaFactories.hpp"
#include "Scripting/API/framework.hpp"

#include "Engine.hpp"
#include "SceneManager.hpp"
#include "lua.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

namespace Engine::Scripting::API
{
	namespace Functions_Engine
	{
		static int GetAssetManager(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 1)
			CMEP_LUAGET_PTR(state, Engine)

			std::weak_ptr<AssetManager> asset_manager = self->getAssetManager();

			if (!asset_manager.expired())
			{
				API::LuaFactories::AssetManagerFactory(state, asset_manager);

				return 1;
			}

			return luaL_error(state, "AssetManager is expired");
		}

		static int GetSceneManager(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 1)
			CMEP_LUAGET_PTR(state, Engine)

			std::weak_ptr<SceneManager> scene_manager = self->getSceneManager();

			if (auto locked_scene_manager = scene_manager.lock())
			{
				API::LuaFactories::SceneManagerFactory(state, locked_scene_manager.get());

				return 1;
			}

			return luaL_error(state, "SceneManager is expired");
		}

		static int SetFramerateTarget(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 2)
			CMEP_LUAGET_PTR(state, Engine)

			auto framerate_target = static_cast<uint_fast16_t>(lua_tointeger(state, 2));

			self->setFramerateTarget(framerate_target);

			return 0;
		}

		static int Stop(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 1)
			CMEP_LUAGET_PTR(state, Engine)

			self->stop();

			return 0;
		}

	} // namespace Functions_Engine

	std::unordered_map<std::string, const lua_CFunction> engine_mappings = {
		CMEP_LUAMAPPING_DEFINE(Functions_Engine, GetAssetManager),
		CMEP_LUAMAPPING_DEFINE(Functions_Engine, GetSceneManager),
		CMEP_LUAMAPPING_DEFINE(Functions_Engine, SetFramerateTarget),
		CMEP_LUAMAPPING_DEFINE(Functions_Engine, Stop),
	};
} // namespace Engine::Scripting::API
