#include "Scripting/API/AssetManager_API.hpp"

namespace Engine::Scripting::API
{
	namespace Functions_AssetManager
	{
		int GetFont(lua_State *state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<AssetManager> asset_manager = *(std::weak_ptr<AssetManager> *)lua_touserdata(state, -1);

			std::string name = lua_tostring(state, 2);

			if (auto locked_asset_manager = asset_manager.lock())
			{
				std::shared_ptr<Rendering::Font> font = locked_asset_manager->GetFont(name);

				if (font != nullptr)
				{
					// Generate object table
					lua_newtable(state);

					void* ptr = lua_newuserdata(state, sizeof(std::shared_ptr<Rendering::Font>));
					new (ptr) std::shared_ptr<Rendering::Font>(font);
					
					lua_setfield(state, -2, "_smart_ptr");

					return 1;
				}
				else
				{
					return luaL_error(state, "AssetManager returned nullptr; GetFont '%s'", name.c_str());
				}
			}
			else
			{
				return luaL_error(state, "Cannot lock AssetManager, invalid object passed");
			}

			return 1;
		}

		int GetTexture(lua_State *state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<AssetManager> asset_manager = *(std::weak_ptr<AssetManager> *)lua_touserdata(state, -1);

			std::string path = lua_tostring(state, 2);

			if (auto locked_asset_manager = asset_manager.lock())
			{
				Rendering::Texture *texture = locked_asset_manager->GetTexture(std::move(path)).get();

				if (texture != nullptr)
				{
					// Generate object table
					lua_newtable(state);

					Rendering::Texture **ptr = (Rendering::Texture **)lua_newuserdata(state, sizeof(Rendering::Texture *));
					(*ptr) = texture;
					lua_setfield(state, -2, "_pointer");
				}
				else
				{
					return luaL_error(state, "AssetManager returned nullptr; GetTexture '%s'", path.c_str());
				}
			}
			else
			{
				return luaL_error(state, "Cannot lock AssetManager, invalid object passed");
			}

			return 1;
		}

		int GetModel(lua_State *state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<AssetManager> asset_manager = *(std::weak_ptr<AssetManager> *)lua_touserdata(state, -1);

			std::string path = lua_tostring(state, 2);

			if (auto locked_asset_manager = asset_manager.lock())
			{
				std::shared_ptr<Rendering::Mesh> model = locked_asset_manager->GetModel(std::move(path));

				if (model != nullptr)
				{
					// Generate object table
					lua_newtable(state);

					void *ptr = lua_newuserdata(state, sizeof(std::shared_ptr<Rendering::Mesh>));
					new (ptr) std::shared_ptr<Rendering::Mesh>(model);

					lua_setfield(state, -2, "_smart_ptr");
				}
				else
				{
					return luaL_error(state, "AssetManager returned nullptr; GetModel '%s'", path.c_str());
				}
			}
			else
			{
				return luaL_error(state, "Cannot lock AssetManager, invalid object passed");
			}

			return 1;
		}

		int AddTexture(lua_State *state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<AssetManager> asset_manager = *(std::weak_ptr<AssetManager> *)lua_touserdata(state, -1);

			std::string name = lua_tostring(state, 2);
			std::string path = lua_tostring(state, 3);

			if (auto locked_asset_manager = asset_manager.lock())
			{
				locked_asset_manager->AddTexture(std::move(name), std::move(path), Rendering::Texture_InitFiletype::FILE_PNG);
			}
			else
			{
				return luaL_error(state, "Cannot lock AssetManager, invalid object passed");
			}

			return 1;
		}
	}

	std::unordered_map<std::string, lua_CFunction> assetManager_Mappings = {
		CMEP_LUAMAPPING_DEFINE(Functions_AssetManager, GetFont),
		CMEP_LUAMAPPING_DEFINE(Functions_AssetManager, GetTexture),
		CMEP_LUAMAPPING_DEFINE(Functions_AssetManager, GetModel),

		CMEP_LUAMAPPING_DEFINE(Functions_AssetManager, AddTexture),
	};
}