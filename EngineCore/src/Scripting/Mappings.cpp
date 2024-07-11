#include "Scripting/Mappings.hpp"

#include "Assets/AssetManager.hpp"
#include "Assets/Texture.hpp"
#include "Rendering/MeshRenderer.hpp"
#include "Rendering/SupplyData.hpp"
#include "Rendering/TextRenderer.hpp"

#include "Scripting/API/LuaFactories.hpp"
#include "Scripting/API/framework.hpp"

#include "Factories/ObjectFactory.hpp"

#include "Engine.hpp"
#include "SceneManager.hpp"
#include "lua.h"
#include "lua.hpp"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_LUA_MAPPED
#include "Logging/LoggingPrefix.hpp"

#undef CMEP_LUAMAPPING_DEFINE
#define CMEP_LUAMAPPING_DEFINE(mapping) {#mapping, Functions::mapping}

namespace Engine::Scripting::Mappings
{
	namespace Functions
	{
#pragma region Meta

		int MetaLoggerSimpleLog(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 1)

			const char* string = lua_tostring(state, 1);

			std::weak_ptr<Logging::Logger> logger = Scripting::API::LuaFactories::MetaLoggerFactory(state);

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

#pragma region Renderer supply

		static int RendererSupplyText(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 2)

			// lua_getfield(state, 1, "_ptr");
			// auto* renderer = static_cast<Rendering::IRenderer*>(lua_touserdata(state, -1));
			auto* renderer = static_cast<Rendering::IRenderer*>(lua_touserdata(state, 1));

			const char* text = lua_tostring(state, 2);

			Rendering::RendererSupplyData text_supply = {Rendering::RendererSupplyDataType::TEXT, text};
			renderer->SupplyData(text_supply);

			return 0;
		}

		static int RendererSupplyTexture(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 2)

			// lua_getfield(state, 1, "_ptr");
			// auto* renderer = static_cast<Rendering::IRenderer*>(lua_touserdata(state, -1));
			auto* renderer = static_cast<Rendering::IRenderer*>(lua_touserdata(state, 1));

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
			CMEP_LUACHECK_FN_ARGC(state, 3)

			lua_getfield(state, 1, "_ptr");
			auto* scene_manager = static_cast<SceneManager*>(lua_touserdata(state, -1));

			lua_getfield(state, 2, "_smart_ptr");
			std::weak_ptr<AssetManager> asset_manager = *static_cast<std::weak_ptr<AssetManager>*>(
				lua_touserdata(state, -1)
			);
			std::string sprite_name = lua_tostring(state, 3);

			auto& scene = scene_manager->GetSceneCurrent();

			if (auto locked_am = asset_manager.lock())
			{
				Object* obj = ObjectFactory::CreateSpriteObject(scene, locked_am->GetTexture(sprite_name));

				if (obj != nullptr)
				{
					API::LuaFactories::ObjectFactory(state, obj);
					return 1;
				}
				return luaL_error(state, "ObjectFactory returned nullptr");
			}

			return luaL_error(state, "Could not lock AssetManager");
		}

		static int ObjectFactoryCreateTextObject(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 3)

			lua_getfield(state, 1, "_ptr");
			auto* scene_manager = static_cast<SceneManager*>(lua_touserdata(state, -1));

			std::string text = lua_tostring(state, 2);

			lua_getfield(state, 3, "_smart_ptr");
			std::weak_ptr<Rendering::Font> font = *static_cast<std::weak_ptr<Rendering::Font>*>(
				lua_touserdata(state, -1)
			);

			Object* obj = nullptr;
			if (auto locked_font = font.lock())
			{
				auto& scene = scene_manager->GetSceneCurrent();

				obj = ObjectFactory::CreateTextObject(scene, text, locked_font);
			}

			if (obj != nullptr)
			{
				API::LuaFactories::ObjectFactory(state, obj);

				return 1;
			}

			return luaL_error(state, "ObjectFactory returned nullptr");
		}

		static int ObjectFactoryCreateGeneric3DObject(lua_State* state)
		{
			CMEP_LUACHECK_FN_ARGC(state, 3)

			lua_getfield(state, 1, "_ptr");
			auto* scene_manager = static_cast<SceneManager*>(lua_touserdata(state, -1));

			std::shared_ptr<Rendering::Mesh> mesh = std::make_shared<Rendering::Mesh>(scene_manager->GetOwnerEngine());

			mesh->CreateMeshFromObj(std::string(lua_tostring(state, 2)));

			auto& scene = scene_manager->GetSceneCurrent();

			Object* obj = ObjectFactory::CreateGeneric3DObject(scene, mesh);

			if (obj != nullptr)
			{
				API::LuaFactories::ObjectFactory(state, obj);

				return 1;
			}

			return luaL_error(state, "ObjectFactory returned nullptr");
		}
#pragma endregion

	} // namespace Functions

	std::unordered_map<std::string, lua_CFunction> mappings = {
		CMEP_LUAMAPPING_DEFINE(RendererSupplyText),
		CMEP_LUAMAPPING_DEFINE(RendererSupplyTexture),

		CMEP_LUAMAPPING_DEFINE(ObjectFactoryCreateSpriteObject),
		CMEP_LUAMAPPING_DEFINE(ObjectFactoryCreateTextObject),
		CMEP_LUAMAPPING_DEFINE(ObjectFactoryCreateGeneric3DObject)
	};
} // namespace Engine::Scripting::Mappings
