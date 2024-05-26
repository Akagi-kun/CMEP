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
		lua_getfield(state, -1, "_smart_pointer");
		std::weak_ptr<Logging::Logger> logger = *(std::weak_ptr<Logging::Logger>*)lua_touserdata(state, -1);

		if(auto locked_logger = logger.lock())
		{
			locked_logger->SimpleLog(Logging::LogLevel::Info, "Lua: %s", string);
		}

		return 0;
	}

#pragma endregion


#pragma region Engine

		int engine_GetAssetManager(lua_State* state)
		{
			Engine* engine = *(Engine**)lua_touserdata(state, 1);

			AssetManager* asset_manager = engine->GetAssetManager();

			if (asset_manager != nullptr)
			{
				// Generate asset_manager table
				lua_newtable(state);

				AssetManager** ptr_obj = (AssetManager**)lua_newuserdata(state, sizeof(AssetManager*));
				(*ptr_obj) = asset_manager;
				lua_setfield(state, -2, "_pointer");
			}
			else
			{
				lua_getglobal(state, "cmepmeta");
				lua_getfield(state, -1, "logger");
				lua_getfield(state, -1, "_smart_pointer");
				std::weak_ptr<Logging::Logger> logger = *(std::weak_ptr<Logging::Logger>*)lua_touserdata(state, -1);

				if(auto locked_logger = logger.lock())
				{
					locked_logger->SimpleLog(Logging::LogLevel::Warning, "Lua: AssetManager requested but returned nullptr!");
				}
				lua_pushnil(state);
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

				// Generate scene_manager table
				//lua_newtable(state);

				//void* ptr_obj = lua_newuserdata(state, sizeof(std::weak_ptr<SceneManager>));
				////(*ptr_obj) = scene_manager;
				//new(ptr_obj) std::weak_ptr<SceneManager>(scene_manager);
				//
				//lua_setfield(state, -2, "_smart_pointer");
			}
			else
			{
				lua_getglobal(state, "cmepmeta");
				lua_getfield(state, -1, "logger");
				lua_getfield(state, -1, "_smart_pointer");
				std::weak_ptr<Logging::Logger> logger = *(std::weak_ptr<Logging::Logger>*)lua_touserdata(state, -1);

				if(auto locked_logger = logger.lock())
				{
					locked_logger->SimpleLog(Logging::LogLevel::Warning, "Lua: SceneManager requested but is expired!");
				}
				lua_pushnil(state);
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



#pragma region AssetManager

		int assetManager_GetFont(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");
			AssetManager* ptr_am = *(AssetManager**)lua_touserdata(state, -1);

			std::string path = lua_tostring(state, 2);

			std::shared_ptr<Rendering::Font> font = ptr_am->GetFont(std::move(path));

			if (font != nullptr)
			{
				// Generate object table
				lua_newtable(state);

				Rendering::Font** ptr = (Rendering::Font**)lua_newuserdata(state, sizeof(Rendering::Font*));
				(*ptr) = font.get();
				lua_setfield(state, -2, "_pointer");
			}
			else
			{
				lua_pushnil(state);
			}

			return 1;
		}

		int assetManager_GetTexture(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");
			AssetManager* ptr_am = *(AssetManager**)lua_touserdata(state, -1);

			std::string path = lua_tostring(state, 2);

			Rendering::Texture* texture = ptr_am->GetTexture(std::move(path)).get();

			if (texture != nullptr)
			{
				// Generate object table
				lua_newtable(state);

				Rendering::Texture** ptr = (Rendering::Texture**)lua_newuserdata(state, sizeof(Rendering::Texture*));
				(*ptr) = texture;
				lua_setfield(state, -2, "_pointer");
			}
			else
			{
				lua_pushnil(state);
			}

			return 1;
		}

		int assetManager_AddTexture(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");
			AssetManager* ptr_am = *(AssetManager**)lua_touserdata(state, -1);

			std::string name = lua_tostring(state, 2);
			std::string path = lua_tostring(state, 3);

			ptr_am->AddTexture(std::move(name), std::move(path), Rendering::Texture_InitFiletype::FILE_PNG);

			return 1;
		}

		int assetManager_GetModel(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");
			AssetManager* ptr_am = *(AssetManager**)lua_touserdata(state, -1);

			std::string path = lua_tostring(state, 2);

			std::shared_ptr<Rendering::Mesh> model = ptr_am->GetModel(std::move(path));

			if (model != nullptr)
			{
				// Generate object table
				lua_newtable(state);

				void* ptr = lua_newuserdata(state, sizeof(std::shared_ptr<Rendering::Mesh>));
				//(*ptr) = model.get();
				
				new(ptr) std::shared_ptr<Rendering::Mesh>(model);
				
				lua_setfield(state, -2, "_pointer");
			}
			else
			{
				lua_pushnil(state);
			}

			return 1;
		}

#pragma endregion



#pragma region ObjectFactory

		int objectFactory_CreateSpriteObject(lua_State* state)
		{
			lua_getfield(state, 1, "_smart_pointer");
			std::weak_ptr<SceneManager> scene_manager = *(std::weak_ptr<SceneManager>*)lua_touserdata(state, -1);

			double x = lua_tonumber(state, 2);
			double y = lua_tonumber(state, 3);
			double sizex = lua_tonumber(state, 4);
			double sizey = lua_tonumber(state, 5);

			lua_getfield(state, 6, "_pointer");
			AssetManager* ptr_am = *(AssetManager**)lua_touserdata(state, -1);
			std::string sprite_name = lua_tostring(state, 7);
			//Rendering::Texture* sprite = *(Rendering::Texture**)lua_touserdata(state, -1);

			Object* obj = ObjectFactory::CreateSpriteObject(scene_manager, x, y, sizex, sizey, ptr_am->GetTexture(sprite_name));

			if (obj != nullptr)
			{
				// Generate object table
				lua_newtable(state);

				Object** ptr_obj = (Object**)lua_newuserdata(state, sizeof(Object*));
				(*ptr_obj) = obj;
				lua_setfield(state, -2, "_pointer");

				// Generate renderer table
				lua_newtable(state);
				Rendering::IRenderer** ptr_renderer = (Rendering::IRenderer**)lua_newuserdata(state, sizeof(Rendering::IRenderer*));
				(*ptr_renderer) = obj->renderer;
				lua_setfield(state, -2, "_pointer");
				lua_setfield(state, -2, "renderer");
			}
			else
			{
				lua_getglobal(state, "cmepmeta");
				lua_getfield(state, -1, "logger");
				lua_getfield(state, -1, "_smart_pointer");
				std::weak_ptr<Logging::Logger> logger = *(std::weak_ptr<Logging::Logger>*)lua_touserdata(state, -1);

				if(auto locked_logger = logger.lock())
				{
					locked_logger->SimpleLog(Logging::LogLevel::Warning, "Lua: Object creation failed, ObjectFactory::CreateSpriteObject returned nullptr! Params: %f %f %f %f", x, y, sizex, sizey);
				}

				lua_pushnil(state);
			}

			return 1;
		}

		int objectFactory_CreateTextObject(lua_State* state)
		{
			lua_getfield(state, 1, "_smart_pointer");
			std::weak_ptr<SceneManager> scene_manager = *(std::weak_ptr<SceneManager>*)lua_touserdata(state, -1);

			double x = lua_tonumber(state, 2);
			double y = lua_tonumber(state, 3);
			int size = static_cast<int>(lua_tointeger(state, 4));

			std::string text = lua_tostring(state, 5);

			lua_getfield(state, 6, "_pointer");
			Rendering::Font* font = *(Rendering::Font**)lua_touserdata(state, -1);

			Object* obj = ObjectFactory::CreateTextObject(scene_manager, x, y, size, text, font);

			if (obj != nullptr)
			{
				// Generate object table
				lua_newtable(state);

				Object** ptr_obj = (Object**)lua_newuserdata(state, sizeof(Object*));
				(*ptr_obj) = obj;
				lua_setfield(state, -2, "_pointer");

				// Generate renderer table
				lua_newtable(state);
				Rendering::IRenderer** ptr_renderer = (Rendering::IRenderer**)lua_newuserdata(state, sizeof(Rendering::IRenderer*));
				(*ptr_renderer) = obj->renderer;
				lua_setfield(state, -2, "_pointer");
				lua_setfield(state, -2, "renderer");
			}
			else
			{
				lua_getglobal(state, "cmepmeta");
				lua_getfield(state, -1, "logger");
				lua_getfield(state, -1, "_smart_pointer");
				std::weak_ptr<Logging::Logger> logger = *(std::weak_ptr<Logging::Logger>*)lua_touserdata(state, -1);

				if(auto locked_logger = logger.lock())
				{
					locked_logger->SimpleLog(Logging::LogLevel::Warning, "Lua: Object creation failed, ObjectFactory::CreateTextObject returned nullptr! Params: %f %f %u '%s'", x, y, size, text.c_str());
				}

				lua_pushnil(state);
			}

			return 1;
		}

		int objectFactory_CreateGeneric3DObject(lua_State* state)
		{
			lua_getfield(state, 1, "_smart_pointer");
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
				// Generate object table
				lua_newtable(state);

				Object** ptr_obj = (Object**)lua_newuserdata(state, sizeof(Object*));
				(*ptr_obj) = obj;
				lua_setfield(state, -2, "_pointer");

				// Generate renderer table
				lua_newtable(state);
				Rendering::IRenderer** ptr_renderer = (Rendering::IRenderer**)lua_newuserdata(state, sizeof(Rendering::IRenderer*));
				(*ptr_renderer) = obj->renderer;
				lua_setfield(state, -2, "_pointer");
				lua_setfield(state, -2, "renderer");
			}
			else
			{
				lua_pushnil(state);
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

		//CMEP_LUAMAPPING_DEFINE(object_AddChild),
		//CMEP_LUAMAPPING_DEFINE(object_GetSize),
		//CMEP_LUAMAPPING_DEFINE(object_Scale),
		//CMEP_LUAMAPPING_DEFINE(object_GetRotation),
		//CMEP_LUAMAPPING_DEFINE(object_Rotate),
		//CMEP_LUAMAPPING_DEFINE(object_GetPosition),
		//CMEP_LUAMAPPING_DEFINE(object_Translate),

		CMEP_LUAMAPPING_DEFINE(assetManager_GetFont),
		CMEP_LUAMAPPING_DEFINE(assetManager_GetTexture),
		CMEP_LUAMAPPING_DEFINE(assetManager_AddTexture),
		CMEP_LUAMAPPING_DEFINE(assetManager_GetModel),

		CMEP_LUAMAPPING_DEFINE(objectFactory_CreateSpriteObject),
		CMEP_LUAMAPPING_DEFINE(objectFactory_CreateTextObject),
		CMEP_LUAMAPPING_DEFINE(objectFactory_CreateGeneric3DObject)
	};
}