#include "Scripting/API/AssetManager_API.hpp"

#include "Assets/AssetManager.hpp"

#include "Scripting/API/framework.hpp"

#include <memory>

namespace Engine::Scripting::API
{
	namespace Functions_AssetManager
	{
		/*
		static int AddTexture(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 3)
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<AssetManager> asset_manager = *static_cast<std::weak_ptr<AssetManager>*>(
				lua_touserdata(state, -1)
			);

			std::string name = lua_tostring(state, 2);
			std::string path = lua_tostring(state, 3);

			if (auto locked_asset_manager = asset_manager.lock())
			{
				static auto texture_factory = Factories::TextureFactory(
					locked_asset_manager->GetOwnerEngine()
				);

				locked_asset_manager->AddTexture(
					name,
					texture_factory.InitFile(path, Rendering::Texture_InitFiletype::FILE_PNG)
				);

				return 1;
			}

			return luaL_error(state, "Cannot lock AssetManager, invalid object passed");
		} */

		//
		static int GetFont(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 2)

			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<AssetManager> asset_manager = *static_cast<std::weak_ptr<AssetManager>*>(
				lua_touserdata(state, -1)
			);

			std::string name = lua_tostring(state, 2);

			if (auto locked_asset_manager = asset_manager.lock())
			{
				auto asset = locked_asset_manager->GetFont(name);

				void* ud_ptr = lua_newuserdata(state, sizeof(std::weak_ptr<Rendering::Font>));
				new (ud_ptr) std::weak_ptr<Rendering::Font>(asset);

				return 1;
			}

			return luaL_error(state, "Could not lock asset manager");
		}

		static int GetTexture(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 2)

			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<AssetManager> asset_manager = *static_cast<std::weak_ptr<AssetManager>*>(
				lua_touserdata(state, -1)
			);

			std::string name = lua_tostring(state, 2);

			if (auto locked_asset_manager = asset_manager.lock())
			{
				auto asset = locked_asset_manager->GetTexture(name);

				void* ud_ptr = lua_newuserdata(state, sizeof(std::weak_ptr<Rendering::Texture>));
				new (ud_ptr) std::weak_ptr<Rendering::Texture>(asset);

				return 1;
			}

			return luaL_error(state, "Could not lock asset manager");
		}

		static int GetScript(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 2)

			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<AssetManager> asset_manager = *static_cast<std::weak_ptr<AssetManager>*>(
				lua_touserdata(state, -1)
			);

			std::string name = lua_tostring(state, 2);

			if (auto locked_asset_manager = asset_manager.lock())
			{
				auto asset = locked_asset_manager->GetLuaScript(name);

				void* ud_ptr = lua_newuserdata(state, sizeof(std::weak_ptr<ILuaScript>));
				new (ud_ptr) std::weak_ptr<ILuaScript>(asset);

				return 1;
			}

			return luaL_error(state, "Could not lock asset manager");
		}
	} // namespace Functions_AssetManager

	std::unordered_map<std::string, const lua_CFunction> asset_manager_mappings = {
		CMEP_LUAMAPPING_DEFINE(Functions_AssetManager, GetFont),
		CMEP_LUAMAPPING_DEFINE(Functions_AssetManager, GetTexture),
		CMEP_LUAMAPPING_DEFINE(Functions_AssetManager, GetScript)

		// CMEP_LUAMAPPING_DEFINE(Functions_AssetManager, AddTexture),
	};
} // namespace Engine::Scripting::API
