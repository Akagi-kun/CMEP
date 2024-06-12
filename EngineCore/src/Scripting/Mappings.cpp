#include "Scripting/Mappings.hpp"
#include "Scripting/API/LuaFactories.hpp"

#include "Rendering/MeshRenderer.hpp"
#include "Rendering/TextRenderer.hpp"
#include "Rendering/Texture.hpp"

#include "AssetManager.hpp"
#include "SceneManager.hpp"

#include "Factories/ObjectFactory.hpp"

#include "Engine.hpp"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_LUA_MAPPED
#include "Logging/LoggingPrefix.hpp"

#define CMEP_LUAMAPPING_DEFINE(mapping) {#mapping, Functions::mapping}

namespace Engine::Scripting::Mappings
{
	namespace Functions
	{
#pragma region Meta

		int MetaLoggerSimpleLog(lua_State* state)
		{
			const char* string = lua_tostring(state, 1);

			lua_getglobal(state, "cmepmeta");
			lua_getfield(state, -1, "logger");
			lua_getfield(state, -1, "_smart_ptr");
			std::weak_ptr<Logging::Logger> logger = *(std::weak_ptr<Logging::Logger>*)lua_touserdata(state, -1);

			if (auto locked_logger = logger.lock())
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

#pragma region TextRenderer

		int TextRendererUpdateText(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");
			Rendering::IRenderer* renderer = *(Rendering::IRenderer**)lua_touserdata(state, -1);

			const char* text = lua_tostring(state, 2);

			Rendering::RendererSupplyData text_supply(Rendering::RendererSupplyDataType::TEXT, text);
			renderer->SupplyData(text_supply);

			//((::Engine::Rendering::TextRenderer*)renderer)->UpdateText(std::string(text));

			return 0;
		}

#pragma endregion

#pragma region MeshRenderer

		int MeshRendererUpdateTexture(lua_State* state)
		{
			lua_getfield(state, 1, "_pointer");
			Rendering::IRenderer* renderer = *(Rendering::IRenderer**)lua_touserdata(state, -1);

			lua_getfield(state, 2, "_smart_ptr");
			std::shared_ptr<Rendering::Texture> texture = *(std::shared_ptr<Rendering::Texture>*)lua_touserdata(
				state, -1
			);

			Rendering::RendererSupplyData texture_supply(Rendering::RendererSupplyDataType::TEXTURE, texture);
			renderer->SupplyData(texture_supply);
			//((::Engine::Rendering::MeshRenderer*)renderer)->UpdateTexture(texture);

			return 0;
		}

#pragma endregion

#pragma region ObjectFactory

		int ObjectFactoryCreateSpriteObject(lua_State* state)
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
			// Rendering::Texture* sprite = *(Rendering::Texture**)lua_touserdata(state, -1);

			Object* obj = ObjectFactory::CreateSpriteObject(
				scene_manager, x, y, z, sizex, sizey, ptr_am->GetTexture(sprite_name)
			);

			if (obj != nullptr)
			{
				API::LuaObjectFactories::ObjectFactory(state, obj);
			}
			else
			{
				std::weak_ptr<Logging::Logger> logger = API::LuaObjectFactories::MetaLoggerFactory(state);

				if (auto locked_logger = logger.lock())
				{
					locked_logger->SimpleLog(
						Logging::LogLevel::Warning,
						"Lua: Object creation failed, ObjectFactory::CreateSpriteObject returned nullptr! Params: %f "
						"%f %f %f",
						x,
						y,
						sizex,
						sizey
					);
				}

				return luaL_error(state, "ObjectFactory returned nullptr");
			}

			return 1;
		}

		int ObjectFactoryCreateTextObject(lua_State* state)
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
				std::weak_ptr<Logging::Logger> logger = API::LuaObjectFactories::MetaLoggerFactory(state);

				if (auto locked_logger = logger.lock())
				{
					locked_logger->SimpleLog(
						Logging::LogLevel::Warning,
						"Lua: Object creation failed, ObjectFactory::CreateTextObject returned nullptr! Params: %f %f "
						"%u '%s'",
						x,
						y,
						size,
						text.c_str()
					);
				}

				return luaL_error(state, "ObjectFactory returned nullptr");
			}

			return 1;
		}

		int ObjectFactoryCreateGeneric3DObject(lua_State* state)
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

			Object* obj = ObjectFactory::CreateGeneric3DObject(
				scene_manager, x, y, z, xsize, ysize, zsize, xrot, yrot, zrot, mesh
			);

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

	} // namespace Functions

	std::unordered_map<std::string, lua_CFunction> mappings = {
		CMEP_LUAMAPPING_DEFINE(TextRendererUpdateText),

		CMEP_LUAMAPPING_DEFINE(MeshRendererUpdateTexture),

		CMEP_LUAMAPPING_DEFINE(ObjectFactoryCreateSpriteObject),
		CMEP_LUAMAPPING_DEFINE(ObjectFactoryCreateTextObject),
		CMEP_LUAMAPPING_DEFINE(ObjectFactoryCreateGeneric3DObject)
	};
} // namespace Engine::Scripting::Mappings
