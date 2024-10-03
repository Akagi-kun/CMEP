#include "Scripting/API/AssetManager_API.hpp"

#include "Assets/AssetManager.hpp"
#include "Assets/Texture.hpp"

#include "Scripting/API/framework.hpp"
#include "Scripting/ILuaScript.hpp"

#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>

namespace Engine::Scripting::API
{
	namespace
	{
		/* int addTexture(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 3)
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<AssetManager> asset_manager =
				*static_cast<std::weak_ptr<AssetManager>*>(lua_touserdata(state, -1));

			std::string name = lua_tostring(state, 2);
			std::string path = lua_tostring(state, 3);

			auto locked_asset_manager = asset_manager.lock();
			assert(locked_asset_manager);

			static auto texture_factory =
				Factories::TextureFactory(locked_asset_manager->getOwnerEngine());

			locked_asset_manager->addTexture(
				name,
				texture_factory.InitFile(path, Rendering::Texture_InitFiletype::FILE_PNG)
			);

			return 1;
		} */

		//
		int getFont(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 2)

			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<AssetManager> asset_manager =
				*static_cast<std::weak_ptr<AssetManager>*>(lua_touserdata(state, -1));

			std::string name = lua_tostring(state, 2);

			auto locked_asset_manager = asset_manager.lock();
			assert(locked_asset_manager);

			auto asset = locked_asset_manager->getFont(name);

			void* ud_ptr = lua_newuserdata(state, sizeof(std::weak_ptr<Rendering::Font>));
			new (ud_ptr) std::weak_ptr<Rendering::Font>(asset);

			return 1;
		}

		int getTexture(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 2)

			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<AssetManager> asset_manager =
				*static_cast<std::weak_ptr<AssetManager>*>(lua_touserdata(state, -1));

			std::string name = lua_tostring(state, 2);

			auto locked_asset_manager = asset_manager.lock();
			assert(locked_asset_manager);

			auto asset = locked_asset_manager->getTexture(name);

			void* ud_ptr =
				lua_newuserdata(state, sizeof(std::weak_ptr<Rendering::Texture>));
			new (ud_ptr) std::weak_ptr<Rendering::Texture>(asset);

			return 1;
		}

		int getScript(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 2)

			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<AssetManager> asset_manager =
				*static_cast<std::weak_ptr<AssetManager>*>(lua_touserdata(state, -1));

			std::string name = lua_tostring(state, 2);

			auto locked_asset_manager = asset_manager.lock();
			assert(locked_asset_manager);

			auto asset = locked_asset_manager->getLuaScript(name);

			void* ud_ptr = lua_newuserdata(state, sizeof(std::weak_ptr<ILuaScript>));
			new (ud_ptr) std::weak_ptr<ILuaScript>(asset);

			return 1;
		}
	} // namespace

	std::unordered_map<std::string, const lua_CFunction> asset_manager_mappings = {
		CMEP_LUAMAPPING_DEFINE(getFont),
		CMEP_LUAMAPPING_DEFINE(getTexture),
		CMEP_LUAMAPPING_DEFINE(getScript)

		// CMEP_LUAMAPPING_DEFINE(Functions_AssetManager, AddTexture),
	};
} // namespace Engine::Scripting::API
