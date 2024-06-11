#pragma once

#include "lualib/lua.hpp"
#include <unordered_map>

namespace Engine::Scripting::Mappings
{
	namespace Functions
	{
		int MetaLoggerSimpleLog(lua_State* state);

		int SmGetCameraHvRotation(lua_State* state);
		int SmSetCameraHvRotation(lua_State* state);
		int SmGetCameraTransform(lua_State* state);
		int SmSetCameraTransform(lua_State* state);
		int SmGetLightTransform(lua_State* state);
		int SmSetLightTransform(lua_State* state);
		int SmAddObject(lua_State* state);
		int SmFindObject(lua_State* state);
		int SmRemoveObject(lua_State* state);

		int EngineGetAssetManager(lua_State* state);
		int EngineSetFramerateTarget(lua_State* state);

		int TextRendererUpdateText(lua_State* state);

		int MeshRendererUpdateTexture(lua_State* state);

		int ObjectGetRotation(lua_State* state);

		int AssetManagerGetFont(lua_State* state);
		int AssetManagerGetTexture(lua_State* state);
		int AssetManagerAddTexture(lua_State* state);

		int ObjectFactoryCreateSpriteObject(lua_State* state);
		int ObjectFactoryCreateTextObject(lua_State* state);
		int ObjectFactoryCreateGeneric3DObject(lua_State* state);

		// int mesh_Mesh(lua_State* state);
		// int mesh_CreateMeshFromObj(lua_State* state);
	} // namespace Functions

	extern std::unordered_map<std::string, lua_CFunction> scene_manager_mappings;
	extern std::unordered_map<std::string, lua_CFunction> object_mappings;

	[[deprecated]]
	extern std::unordered_map<std::string, lua_CFunction> mappings;

} // namespace Engine::Scripting::Mappings
