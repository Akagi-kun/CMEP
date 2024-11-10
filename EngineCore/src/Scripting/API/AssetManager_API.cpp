#include "Scripting/API/AssetManager_API.hpp"

#include "Assets/AssetManager.hpp"
#include "Assets/Font.hpp"
#include "Assets/Texture.hpp"

#include "Scripting/API/framework.hpp"
#include "Scripting/ILuaScript.hpp"

#include "Exception.hpp"

#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>

namespace Engine::Scripting::API
{
	/// @cond LUA_API
	namespace
	{
		int getFont(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 2)

			auto* asset_manager = getObjectAsPointer<AssetManager>(state, 1);

			std::string name = lua_tostring(state, 2);

			auto asset = UNWRAP(asset_manager->getAsset<Rendering::Font>(name));

			void* ud_ptr = lua_newuserdata(state, sizeof(std::weak_ptr<Rendering::Font>));
			new (ud_ptr) std::weak_ptr<Rendering::Font>(asset);

			return 1;
		}

		int getTexture(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 2)

			auto* asset_manager = getObjectAsPointer<AssetManager>(state, 1);

			std::string name = lua_tostring(state, 2);

			auto asset = UNWRAP(asset_manager->getAsset<Rendering::Texture>(name));

			void* ud_ptr = lua_newuserdata(state, sizeof(std::weak_ptr<Rendering::Texture>));
			new (ud_ptr) std::weak_ptr<Rendering::Texture>(asset);

			return 1;
		}

		int getScript(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 2)

			auto* asset_manager = getObjectAsPointer<AssetManager>(state, 1);

			std::string name = lua_tostring(state, 2);

			auto asset = UNWRAP(asset_manager->getAsset<ILuaScript>(name));

			void* ud_ptr = lua_newuserdata(state, sizeof(std::weak_ptr<ILuaScript>));
			new (ud_ptr) std::weak_ptr<ILuaScript>(asset);

			return 1;
		}
	} // namespace
	/// @endcond

	std::unordered_map<std::string, const lua_CFunction> asset_manager_mappings = {
		CMEP_LUAMAPPING_DEFINE(getFont),
		CMEP_LUAMAPPING_DEFINE(getTexture),
		CMEP_LUAMAPPING_DEFINE(getScript)
	};
} // namespace Engine::Scripting::API
