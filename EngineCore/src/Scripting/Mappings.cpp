#include "Scripting/Mappings.hpp"

#include "Assets/AssetManager.hpp"
#include "Assets/Texture.hpp"
#include "Rendering/MeshRenderer.hpp"
#include "Rendering/TextRenderer.hpp"
// #include "Rendering/Transform.hpp"

#include "Scripting/API/LuaFactories.hpp"
// #include "Scripting/lualib/lua.h"
#include "Factories/ObjectFactory.hpp"

#include "Engine.hpp"
#include "IModule.hpp"
#include "SceneManager.hpp"
#include "lua.hpp"

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
			assert(lua_gettop(state) == 1);

			const char* string = lua_tostring(state, 1);

			std::weak_ptr<Logging::Logger> logger = Scripting::API::LuaObjectFactories::MetaLoggerFactory(state);

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
			assert(lua_gettop(state) == 2);

			lua_getfield(state, 1, "_pointer");
			auto* renderer = static_cast<Rendering::IRenderer*>(lua_touserdata(state, -1));

			const char* text = lua_tostring(state, 2);

			// TODO: Communicate using IModule or even Object?
			ModuleMessage text_supply_message = {
				ModuleMessageType::RENDERER_SUPPLY,
				Rendering::RendererSupplyData{Rendering::RendererSupplyDataType::TEXT, text}
			};
			renderer->Communicate(text_supply_message);
			// renderer->SupplyData(text_supply);

			return 0;
		}

#pragma endregion

#pragma region MeshRenderer

		int MeshRendererUpdateTexture(lua_State* state)
		{
			assert(lua_gettop(state) == 2);

			lua_getfield(state, 1, "_pointer");
			auto* renderer = static_cast<Rendering::IRenderer*>(lua_touserdata(state, -1));

			lua_getfield(state, 2, "_smart_ptr");
			std::shared_ptr<Rendering::Texture> texture = *static_cast<std::shared_ptr<Rendering::Texture>*>(
				lua_touserdata(state, -1)
			);

			// TODO: Communicate using IModule or even Object?
			ModuleMessage texture_supply_message = {
				ModuleMessageType::RENDERER_SUPPLY,
				Rendering::RendererSupplyData{Rendering::RendererSupplyDataType::TEXTURE, texture}
			};
			renderer->Communicate(texture_supply_message);
			// renderer->SupplyData(texture_supply);

			return 0;
		}

#pragma endregion

#pragma region ObjectFactory

		int ObjectFactoryCreateSpriteObject(lua_State* state)
		{
			assert(lua_gettop(state) == 8);

			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *static_cast<std::weak_ptr<SceneManager>*>(
				lua_touserdata(state, -1)
			);

			double position_x = lua_tonumber(state, 2);
			double position_y = lua_tonumber(state, 3);
			double position_z = lua_tonumber(state, 4);
			double size_x	  = lua_tonumber(state, 5);
			double size_y	  = lua_tonumber(state, 6);

			lua_getfield(state, 7, "_pointer");
			AssetManager* ptr_am	= *static_cast<AssetManager**>(lua_touserdata(state, -1));
			std::string sprite_name = lua_tostring(state, 8);

			Object* obj = ObjectFactory::CreateSpriteObject(
				scene_manager,
				position_x,
				position_y,
				position_z,
				size_x,
				size_y,
				ptr_am->GetTexture(sprite_name)
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
						position_x,
						position_y,
						size_x,
						size_y
					);
				}

				return luaL_error(state, "ObjectFactory returned nullptr");
			}

			return 1;
		}

		int ObjectFactoryCreateTextObject(lua_State* state)
		{
			assert(lua_gettop(state) == 7);

			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *static_cast<std::weak_ptr<SceneManager>*>(
				lua_touserdata(state, -1)
			);

			double position_x = lua_tonumber(state, 2);
			double position_y = lua_tonumber(state, 3);
			double position_z = lua_tonumber(state, 4);
			int size		  = static_cast<int>(lua_tointeger(state, 5));

			std::string text = lua_tostring(state, 6);

			lua_getfield(state, 7, "_smart_ptr");
			std::weak_ptr<Rendering::Font> font = *static_cast<std::weak_ptr<Rendering::Font>*>(
				lua_touserdata(state, -1)
			);

			Object* obj = nullptr;
			if (auto locked_font = font.lock())
			{
				obj = ObjectFactory::CreateTextObject(
					scene_manager,
					position_x,
					position_y,
					position_z,
					size,
					text,
					locked_font
				);
			}

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
						position_x,
						position_y,
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
			assert(lua_gettop(state) == 11);

			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *static_cast<std::weak_ptr<SceneManager>*>(
				lua_touserdata(state, -1)
			);

			double position_x = lua_tonumber(state, 2);
			double position_y = lua_tonumber(state, 3);
			double position_z = lua_tonumber(state, 4);
			double size_x	  = lua_tonumber(state, 5);
			double size_y	  = lua_tonumber(state, 6);
			double size_z	  = lua_tonumber(state, 7);
			double rotation_x = lua_tonumber(state, 8);
			double rotation_y = lua_tonumber(state, 9);
			double rotation_z = lua_tonumber(state, 10);

			std::shared_ptr<Rendering::Mesh> mesh = std::make_shared<Rendering::Mesh>();
			mesh->CreateMeshFromObj(std::string(lua_tostring(state, 11)));

			Object* obj = ObjectFactory::CreateGeneric3DObject(
				scene_manager,
				position_x,
				position_y,
				position_z,
				size_x,
				size_y,
				size_z,
				rotation_x,
				rotation_y,
				rotation_z,
				mesh
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
