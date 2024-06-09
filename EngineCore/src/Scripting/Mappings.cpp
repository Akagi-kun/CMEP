#include "glm/glm.hpp"
#include "Rendering/TextRenderer.hpp"
#include "Rendering/MeshRenderer.hpp"
#include "Scripting/Mappings.hpp"
#include "Rendering/Texture.hpp"

#include "SceneManager.hpp"
#include "AssetManager.hpp"

#include "Factories/ObjectFactory.hpp"

#include "Engine.hpp"

#include "Scripting/API/LuaFactories.hpp"
#include "Scripting/API/Object_API.hpp"
#include "Scripting/API/SceneManager_API.hpp"

#ifdef CMEP_LUAMAPPING_DEFINE
#undef CMEP_LUAMAPPING_DEFINE
#define CMEP_LUAMAPPING_DEFINE(mapping) {#mapping, Functions::mapping }
#endif

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_LUA_MAPPED
#include "Logging/LoggingPrefix.hpp"

namespace Engine::Scripting::Mappings
{
	namespace Functions
	{
#pragma region Meta

	int metaLogger_SimpleLog(lua_State* state)
	{
		const char* string = lua_tostring(state, 1);

		lua_getglobal(state, "cmepmeta");
		lua_getfield(state, -1, "logger");
		lua_getfield(state, -1, "_smart_ptr");
		std::weak_ptr<Logging::Logger> logger = *(std::weak_ptr<Logging::Logger>*)lua_touserdata(state, -1);

		if(auto locked_logger = logger.lock())
		{
			locked_logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "%s", string);
		}
		else
		{
			return luaL_error(state, "Could not lock global meta logger");
		}

		return 0;
	}

#pragma endregion



#pragma region Engine

		int engine_GetAssetManager(lua_State* state)
		{
			Engine* engine = *(Engine**)lua_touserdata(state, 1);

			std::weak_ptr<AssetManager> asset_manager = engine->GetAssetManager();

			if (!asset_manager.expired())
			{
				API::LuaObjectFactories::AssetManagerFactory(state, asset_manager);
			}
			else
			{
				std::weak_ptr<Logging::Logger> logger = API::LuaObjectFactories::MetaLogger_Factory(state);

				if(auto locked_logger = logger.lock())
				{
					locked_logger->SimpleLog(Logging::LogLevel::Warning, LOGPFX_CURRENT "AssetManager requested but is expired!");
				}

				return luaL_error(state, "AssetManager is expired");
			}

			return 1;
		}

		int engine_GetSceneManager(lua_State* state)
		{
			Engine* engine = *(Engine**)lua_touserdata(state, 1);

			std::weak_ptr<SceneManager> scene_manager = engine->GetSceneManager();

			if (!scene_manager.expired())
			{
				API::LuaObjectFactories::SceneManagerFactory(state, scene_manager);
			}
			else
			{
				std::weak_ptr<Logging::Logger> logger = API::LuaObjectFactories::MetaLogger_Factory(state);

				if(auto locked_logger = logger.lock())
				{
					locked_logger->SimpleLog(Logging::LogLevel::Warning, LOGPFX_CURRENT "SceneManager requested but is expired!");
				}

				return luaL_error(state, "SceneManager is expired");
			}

			return 1;
		}

		int engine_SetFramerateTarget(lua_State* state)
		{
			Engine* engine = *(Engine**)lua_touserdata(state, 1);

			unsigned int framerate_target = static_cast<unsigned int>(lua_tointeger(state, 2));

			engine->SetFramerateTarget(framerate_target);

			return 0;
		}

		int engine_Stop(lua_State* state)
		{
			Engine* engine = *(Engine**)lua_touserdata(state, 1);

			engine->Stop();
			
			return 0;
		}

#pragma endregion



#pragma region TextRenderer

		int textRenderer_UpdateText(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");
			Rendering::IRenderer* renderer = *(Rendering::IRenderer**)lua_touserdata(state, -1);

			const char* newtext = lua_tostring(state, 2);

			((::Engine::Rendering::TextRenderer*)renderer)->UpdateText(std::string(newtext));

			return 0;
		}

#pragma endregion



#pragma region MeshRenderer

		int meshRenderer_UpdateTexture(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");
			Rendering::IRenderer* renderer = *(Rendering::IRenderer**)lua_touserdata(state, -1);

			lua_getfield(state, 2, "_pointer");
			Rendering::Texture* texture = *(Rendering::Texture**)lua_touserdata(state, -1);

			((::Engine::Rendering::MeshRenderer*)renderer)->UpdateTexture(texture);

			return 0;
		}

#pragma endregion



#pragma region ObjectFactory

		int objectFactory_CreateSpriteObject(lua_State* state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *(std::weak_ptr<SceneManager>*)lua_touserdata(state, -1);

			double x = lua_tonumber(state, 2);
			double y = lua_tonumber(state, 3);
			double z = lua_tonumber(state, 4);
			double sizex = lua_tonumber(state, 5);
			double sizey = lua_tonumber(state, 6);

			lua_getfield(state, 7, "_pointer");
			AssetManager* ptr_am = *(AssetManager**)lua_touserdata(state, -1);
			std::string sprite_name = lua_tostring(state, 8);
			//Rendering::Texture* sprite = *(Rendering::Texture**)lua_touserdata(state, -1);

			Object* obj = ObjectFactory::CreateSpriteObject(scene_manager, x, y, z, sizex, sizey, ptr_am->GetTexture(sprite_name));

			if (obj != nullptr)
			{
				API::LuaObjectFactories::ObjectFactory(state, obj);
			}
			else
			{
				std::weak_ptr<Logging::Logger> logger = API::LuaObjectFactories::MetaLogger_Factory(state);

				if(auto locked_logger = logger.lock())
				{
					locked_logger->SimpleLog(Logging::LogLevel::Warning, "Lua: Object creation failed, ObjectFactory::CreateSpriteObject returned nullptr! Params: %f %f %f %f", x, y, sizex, sizey);
				}

				return luaL_error(state, "ObjectFactory returned nullptr");
			}

			return 1;
		}

		int objectFactory_CreateTextObject(lua_State* state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *(std::weak_ptr<SceneManager>*)lua_touserdata(state, -1);

			double x = lua_tonumber(state, 2);
			double y = lua_tonumber(state, 3);
			double z = lua_tonumber(state, 4);
			int size = static_cast<int>(lua_tointeger(state, 5));

			std::string text = lua_tostring(state, 6);

			lua_getfield(state, 7, "_smart_ptr");
			std::shared_ptr<Rendering::Font> font = *(std::shared_ptr<Rendering::Font>*)lua_touserdata(state, -1);

			Object* obj = ObjectFactory::CreateTextObject(scene_manager, x, y, z, size, text, font);

			if (obj != nullptr)
			{
				API::LuaObjectFactories::ObjectFactory(state, obj);
			}
			else
			{
				std::weak_ptr<Logging::Logger> logger = API::LuaObjectFactories::MetaLogger_Factory(state);

				if(auto locked_logger = logger.lock())
				{
					locked_logger->SimpleLog(Logging::LogLevel::Warning, "Lua: Object creation failed, ObjectFactory::CreateTextObject returned nullptr! Params: %f %f %u '%s'", x, y, size, text.c_str());
				}

				return luaL_error(state, "ObjectFactory returned nullptr");
			}

			return 1;
		}

		int objectFactory_CreateGeneric3DObject(lua_State* state)
		{
			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *(std::weak_ptr<SceneManager>*)lua_touserdata(state, -1);

			double x = lua_tonumber(state, 2);
			double y = lua_tonumber(state, 3);
			double z = lua_tonumber(state, 4);
			double xsize = lua_tonumber(state, 5);
			double ysize = lua_tonumber(state, 6);
			double zsize = lua_tonumber(state, 7);
			double xrot = lua_tonumber(state, 8);
			double yrot = lua_tonumber(state, 9);
			double zrot = lua_tonumber(state, 10);

			std::shared_ptr<Rendering::Mesh> mesh = std::make_shared<Rendering::Mesh>();
			mesh->CreateMeshFromObj(std::string(lua_tostring(state, 11)));

			Object* obj = ObjectFactory::CreateGeneric3DObject(scene_manager, x, y, z, xsize, ysize, zsize, xrot, yrot, zrot, mesh);

			if (obj != nullptr)
			{
				API::LuaObjectFactories::ObjectFactory(state, obj);
			}
			else
			{
				return luaL_error(state, "ObjectFactory returned nullptr");
			}

			return 1;
		}
#pragma endregion
	
	}

	std::unordered_map<std::string, lua_CFunction> mappings = {
		CMEP_LUAMAPPING_DEFINE(engine_GetAssetManager),
		CMEP_LUAMAPPING_DEFINE(engine_SetFramerateTarget),
		CMEP_LUAMAPPING_DEFINE(engine_GetSceneManager),
		CMEP_LUAMAPPING_DEFINE(engine_Stop),

		CMEP_LUAMAPPING_DEFINE(textRenderer_UpdateText),

		CMEP_LUAMAPPING_DEFINE(meshRenderer_UpdateTexture),

		CMEP_LUAMAPPING_DEFINE(objectFactory_CreateSpriteObject),
		CMEP_LUAMAPPING_DEFINE(objectFactory_CreateTextObject),
		CMEP_LUAMAPPING_DEFINE(objectFactory_CreateGeneric3DObject)
	};
}