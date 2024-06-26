#include "Scripting/Mappings.hpp"

#include "Assets/AssetManager.hpp"
#include "Assets/Texture.hpp"
#include "Rendering/MeshRenderer.hpp"
#include "Rendering/SupplyData.hpp"
#include "Rendering/TextRenderer.hpp"
// #include "Rendering/Transform.hpp"

#include "Scripting/API/LuaFactories.hpp"

#include "Factories/ObjectFactory.hpp"

#include "Engine.hpp"
#include "SceneManager.hpp"
#include "lua.h"
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
		/*
		#pragma region TextRenderer

				int TextRendererUpdateText(lua_State* state)
				{
					assert(lua_gettop(state) == 2);

					lua_getfield(state, 1, "_pointer");
					auto* renderer = static_cast<Rendering::IRenderer*>(lua_touserdata(state, -1));

					const char* text = lua_tostring(state, 2);

					Rendering::RendererSupplyData text_supply = {Rendering::RendererSupplyDataType::TEXT, text};
					renderer->SupplyData(text_supply);

					return 0;
				}

		#pragma endregion
		 */
		/*
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

					Rendering::RendererSupplyData texture_supply = {Rendering::RendererSupplyDataType::TEXTURE,
		texture}; renderer->SupplyData(texture_supply);

					return 0;
				}
		#pragma endregion
		*/

#pragma region Renderer supply

		static int RendererSupplyText(lua_State* state)
		{
			assert(lua_gettop(state) == 2);

			lua_getfield(state, 1, "_pointer");
			auto* renderer = static_cast<Rendering::IRenderer*>(lua_touserdata(state, -1));

			const char* text = lua_tostring(state, 2);

			Rendering::RendererSupplyData text_supply = {Rendering::RendererSupplyDataType::TEXT, text};
			renderer->SupplyData(text_supply);

			return 0;
		}

		static int RendererSupplyTexture(lua_State* state)
		{
			assert(lua_gettop(state) == 2);

			lua_getfield(state, 1, "_pointer");
			auto* renderer = static_cast<Rendering::IRenderer*>(lua_touserdata(state, -1));

			lua_getfield(state, 2, "_smart_ptr");
			std::shared_ptr<Rendering::Texture> texture = *static_cast<std::shared_ptr<Rendering::Texture>*>(
				lua_touserdata(state, -1)
			);

			Rendering::RendererSupplyData texture_supply = {Rendering::RendererSupplyDataType::TEXTURE, texture};
			renderer->SupplyData(texture_supply);

			return 0;
		}

#pragma endregion

#pragma region ObjectFactory

		static int ObjectFactoryCreateSpriteObject(lua_State* state)
		{
			assert(lua_gettop(state) == 3);

			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *static_cast<std::weak_ptr<SceneManager>*>(
				lua_touserdata(state, -1)
			);

			lua_getfield(state, 2, "_pointer");
			AssetManager* ptr_am	= *static_cast<AssetManager**>(lua_touserdata(state, -1));
			std::string sprite_name = lua_tostring(state, 3);

			Object* obj = ObjectFactory::CreateSpriteObject(scene_manager, ptr_am->GetTexture(sprite_name));

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

		static int ObjectFactoryCreateTextObject(lua_State* state)
		{
			assert(lua_gettop(state) == 3);

			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *static_cast<std::weak_ptr<SceneManager>*>(
				lua_touserdata(state, -1)
			);

			std::string text = lua_tostring(state, 2);

			lua_getfield(state, 3, "_smart_ptr");
			std::weak_ptr<Rendering::Font> font = *static_cast<std::weak_ptr<Rendering::Font>*>(
				lua_touserdata(state, -1)
			);

			Object* obj = nullptr;
			if (auto locked_font = font.lock())
			{
				obj = ObjectFactory::CreateTextObject(scene_manager, text, locked_font);
			}

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

		static int ObjectFactoryCreateGeneric3DObject(lua_State* state)
		{
			assert(lua_gettop(state) == 2);

			lua_getfield(state, 1, "_smart_ptr");
			std::weak_ptr<SceneManager> scene_manager = *static_cast<std::weak_ptr<SceneManager>*>(
				lua_touserdata(state, -1)
			);

			std::shared_ptr<Rendering::Mesh> mesh = std::make_shared<Rendering::Mesh>();
			mesh->CreateMeshFromObj(std::string(lua_tostring(state, 2)));

			Object* obj = ObjectFactory::CreateGeneric3DObject(scene_manager, mesh);

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
		// CMEP_LUAMAPPING_DEFINE(TextRendererUpdateText),

		// CMEP_LUAMAPPING_DEFINE(MeshRendererUpdateTexture),

		CMEP_LUAMAPPING_DEFINE(RendererSupplyText),
		CMEP_LUAMAPPING_DEFINE(RendererSupplyTexture),

		CMEP_LUAMAPPING_DEFINE(ObjectFactoryCreateSpriteObject),
		CMEP_LUAMAPPING_DEFINE(ObjectFactoryCreateTextObject),
		CMEP_LUAMAPPING_DEFINE(ObjectFactoryCreateGeneric3DObject)
	};
} // namespace Engine::Scripting::Mappings
