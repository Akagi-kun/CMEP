#include "SceneLoader.hpp"

#include "Assets/AssetManager.hpp"
#include "Assets/Texture.hpp"
#include "Rendering/SupplyData.hpp"

#include "Scripting/EventLuaScript.hpp"
#include "Scripting/GeneratorLuaScript.hpp"
#include "Scripting/ILuaScript.hpp"

#include "Factories/FontFactory.hpp"
#include "Factories/ObjectFactory.hpp"
#include "Factories/TextureFactory.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"
#include "EnumStringConvertor.hpp"
#include "EventHandling.hpp"
#include "Exception.hpp"
#include "Scene.hpp"
#include "nlohmann/json.hpp"
#include "vulkan/vulkan_enums.hpp"

#include <cassert>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <memory>
#include <string>

namespace Engine
{
#pragma region Static

	namespace
	{
		template <typename supply_data_t>
		[[nodiscard]] supply_data_t interpretSupplyData(
			AssetManager*				 asset_manager,
			typename supply_data_t::Type type,
			const std::string&			 value
		)
		{
			using namespace Rendering;

			if constexpr (std::is_same_v<supply_data_t, RendererSupplyData>)
			{
				switch (type)
				{
					case RendererSupplyData::Type::FONT:
					{
						return {type, asset_manager->getFont(value)};
					}
					case RendererSupplyData::Type::TEXTURE:
					{
						return {type, asset_manager->getTexture(value)};
					}
					default:
					{
						throw ENGINE_EXCEPTION("Invalid supply data type!");
					}
				}
			}
			else if constexpr (std::is_same_v<supply_data_t, MeshBuilderSupplyData>)
			{
				switch (type)
				{
					case MeshBuilderSupplyData::Type::TEXT:
					{
						return {type, value};
					}
					/**
					 * @todo Generator?
					 */
					case MeshBuilderSupplyData::Type::FONT:
					default:
					{
						throw ENGINE_EXCEPTION("Invalid supply data type!");
					}
				}
			}
		}

	} // namespace

#pragma endregion

#pragma region Public

	SceneLoader::~SceneLoader()
	{
		this->logger->simpleLog<decltype(this)>(
			Logging::LogLevel::VerboseDebug,
			"Destructor called"
		);
	}

	std::shared_ptr<Scene> SceneLoader::loadScene(std::string scene_name)
	{
		std::shared_ptr<Scene> new_scene = std::make_shared<Scene>(owner_engine);

		this->logger->simpleLog<decltype(this)>(
			Logging::LogLevel::Info,
			"Loading scene: '%s'",
			scene_name.c_str()
		);

		loadSceneInternal(new_scene, scene_name);

		return new_scene;
	}

#pragma endregion

#pragma region Protected

	void SceneLoader::loadSceneInternal(std::shared_ptr<Scene>& scene, std::string& scene_name)
	{
		std::filesystem::path scene_path = scene_prefix / scene_name;

		nlohmann::json data;
		try
		{
			std::ifstream file(scene_path / "scene.json");

			data = nlohmann::json::parse(file);

			file.close();
		}
		catch (...)
		{
			std::throw_with_nested(ENGINE_EXCEPTION("Failed on json parse"));
		}

		this->logger->simpleLog<decltype(this)>(
			Logging::LogLevel::VerboseDebug,
			"Loading scene prefix is: %s",
			scene_path.c_str()
		);

		auto asset_manager = owner_engine->getAssetManager().lock();
		ENGINE_EXCEPTION_ON_ASSERT_NOMSG(asset_manager)

		asset_manager->cleanRepository();

		try
		{
			// Load Assets
			loadSceneAssets(asset_manager, data, scene_path);
		}
		catch (...)
		{
			std::throw_with_nested(ENGINE_EXCEPTION("Failed loading assets"));
		}

		try
		{
			// Load Event Handlers
			loadSceneEventHandlers(asset_manager, data, scene);

			// Load Templates
			loadSceneTemplates(asset_manager, data, scene);
		}
		catch (...)
		{
			std::throw_with_nested(ENGINE_EXCEPTION("Failed on post-asset scene load"));
		}
	}

	void SceneLoader::loadSceneEventHandlers(
		std::shared_ptr<AssetManager>& asset_manager,
		nlohmann::json&				   data,
		std::shared_ptr<Scene>&		   scene
	)
	{
		// Load scene event handlers
		for (const auto& event_handler_entry : data["event_handlers"])
		{
			std::string event_handler_type = event_handler_entry["type"].get<std::string>();
			std::string script_name = event_handler_entry["file"].get<std::string>();
			std::string script_function =
				event_handler_entry["function"].get<std::string>();

			EventHandling::EventType event_type =
				EnumStringConvertor<EventHandling::EventType>(event_handler_type);

			std::shared_ptr<Scripting::ILuaScript> event_handler =
				asset_manager->getLuaScript(script_name);

			ENGINE_EXCEPTION_ON_ASSERT(
				event_handler != nullptr,
				std::format(
					"Asset script '{}' required to serve defined event handlers!",
					script_name
				)
			)

			scene->lua_event_handlers.emplace(
				event_type,
				Scripting::ScriptFunctionRef{
					.script	  = event_handler,
					.function = script_function
				}
			);
		}

		this->logger->simpleLog<decltype(this)>(
			Logging::LogLevel::Debug,
			"Done stage: Event Handlers"
		);
	}

	void SceneLoader::loadSceneTemplates(
		std::shared_ptr<AssetManager>& asset_manager,
		nlohmann::json&				   data,
		std::shared_ptr<Scene>&		   scene
	)
	{
		// Load scene object templates
		for (auto& entry : data["templates"])
		{
			Factories::ObjectFactory::ObjectTemplate obj_template{};

			obj_template.with_renderer	   = entry["renderer"].get<std::string>();
			obj_template.with_mesh_builder = entry["mesh_builder"].get<std::string>();
			obj_template.with_shader	   = entry["shader_name"].get<std::string>();

			for (auto& supply_entry : entry["renderer_supply_data"])
			{
				EnumStringConvertor<Rendering::RendererSupplyData::Type> type =
					supply_entry[0].get<std::string>();
				std::string val = supply_entry[1].get<std::string>();

				obj_template.renderer_supply_list.emplace_back(
					interpretSupplyData<Rendering::RendererSupplyData>(
						asset_manager.get(),
						type,
						val
					)
				);
			}

			for (auto& supply_entry : entry["meshbuilder_supply_data"])
			{
				EnumStringConvertor<Rendering::MeshBuilderSupplyData::Type> type =
					supply_entry[0].get<std::string>();
				std::string val = supply_entry[1].get<std::string>();

				obj_template.meshbuilder_supply_list.emplace_back(
					interpretSupplyData<Rendering::MeshBuilderSupplyData>(
						asset_manager.get(),
						type,
						val
					)
				);
			}

			std::string name = entry["name"].get<std::string>();

			scene->loadTemplatedObject(name, obj_template);

			this->logger->simpleLog<decltype(this)>(
				Logging::LogLevel::VerboseDebug,
				"Loaded template '%s'",
				name.c_str()
			);
		}

		this->logger->simpleLog<decltype(this)>(
			Logging::LogLevel::Debug,
			"Done stage: Templates"
		);
	}

	namespace
	{
		template <AssetType type>
		[[nodiscard]] auto loadSceneAssetInternal(
			Engine*						 owner_engine,
			const std::filesystem::path& asset_path,
			nlohmann::json&				 json_entry
		);

		template <>
		[[nodiscard]] auto loadSceneAssetInternal<AssetType::TEXTURE>(
			Engine*						 owner_engine,
			const std::filesystem::path& asset_path,
			nlohmann::json&				 json_entry
		)
		{
			static auto texture_factory = Factories::TextureFactory(owner_engine);

			vk::Filter filtering = vk::Filter::eLinear;
			if (json_entry.contains("filtering"))
			{
				filtering = EnumStringConvertor<vk::Filter>(
					json_entry["filtering"].get<std::string>()
				);
			}

			// Check if a specific sampling is requested, otherwise use default
			vk::SamplerAddressMode sampling_mode = vk::SamplerAddressMode::eRepeat;
			if (json_entry.contains("sampling_mode"))
			{
				sampling_mode = EnumStringConvertor<vk::SamplerAddressMode>(
					json_entry["sampling_mode"].get<std::string>()
				);
			}

			auto texture = texture_factory.initFile(
				asset_path,
				Rendering::Texture_InitFiletype::FILE_PNG,
				filtering,
				sampling_mode
			);

			return texture;
		}

		template <>
		[[nodiscard]] auto loadSceneAssetInternal<AssetType::FONT>(
			Engine*						 owner_engine,
			const std::filesystem::path& asset_path,
			nlohmann::json&				 json_entry
		)
		{
			static auto texture_factory = Factories::TextureFactory(owner_engine);
			static auto font_factory	= Factories::FontFactory(owner_engine);

			// Use a lambda as the callback to createFont
			auto callback = [&](const std::filesystem::path& path) {
				assert(!path.empty());

				auto texture = texture_factory.initFile(
					path,
					Rendering::Texture_InitFiletype::FILE_PNG
				);

				return texture;
			};

			return font_factory.createFont(asset_path, callback);
		}

		template <>
		[[nodiscard]] auto loadSceneAssetInternal<AssetType::SCRIPT>(
			Engine*						 owner_engine,
			const std::filesystem::path& asset_path,
			nlohmann::json&				 json_entry
		)
		{
			if (json_entry.contains("is_generator"))
			{
				return std::shared_ptr<Scripting::ILuaScript>(
					new Scripting::GeneratorLuaScript(owner_engine, asset_path)
				);
			}

			return std::shared_ptr<Scripting::ILuaScript>(
				new Scripting::EventLuaScript(owner_engine, asset_path)
			);
		}

	} // namespace

	void SceneLoader::loadSceneAsset(
		std::shared_ptr<AssetManager>& asset_manager,
		nlohmann::json&				   asset_entry,
		const std::filesystem::path&   scene_path
	)
	{
		// Gets constructed on first call to this function
		static auto font_factory	= Factories::FontFactory(owner_engine);
		static auto texture_factory = Factories::TextureFactory(owner_engine);

		AssetType asset_type =
			EnumStringConvertor<AssetType>(asset_entry["type"].get<std::string>());

		std::string			  asset_name = asset_entry["name"].get<std::string>();
		std::filesystem::path asset_path = scene_path /
										   asset_entry["location"].get<std::string>();

		switch (asset_type)
		{
			case AssetType::TEXTURE:
			{
				auto asset = loadSceneAssetInternal<AssetType::TEXTURE>(
					owner_engine,
					asset_path,
					asset_entry
				);

				asset_manager->addTexture(asset_name, asset);
				break;
			}
			case AssetType::SCRIPT:
			{
				auto asset = loadSceneAssetInternal<AssetType::SCRIPT>(
					owner_engine,
					asset_path,
					asset_entry
				);

				asset_manager->addLuaScript(asset_name, asset);
				break;
			}
			case AssetType::FONT:
			{
				auto asset = loadSceneAssetInternal<AssetType::FONT>(
					owner_engine,
					asset_path,
					asset_entry
				);

				asset_manager->addFont(asset_name, asset);
				break;
			}
			default:
				throw ENGINE_EXCEPTION(std::format(
					"Unknown type '{}' for asset '{}'",
					asset_entry["type"].get<std::string>(),
					asset_name
				));
		}
	}

	void SceneLoader::loadSceneAssets(
		std::shared_ptr<AssetManager>& asset_manager,
		nlohmann::json&				   data,
		const std::filesystem::path&   scene_path
	)
	{
		for (auto& asset_entry : data["assets"])
		{
			try
			{
				loadSceneAsset(asset_manager, asset_entry, scene_path);
			}
			catch (...)
			{
				std::throw_with_nested(ENGINE_EXCEPTION(std::format(
					"Exception occured loading asset! Relevant JSON:\n\t{}",
					asset_entry.dump()
				)));
			}
		}

		this->logger->simpleLog<decltype(this)>(
			Logging::LogLevel::Debug,
			"Done stage: Assets"
		);
	}

#pragma endregion

} // namespace Engine
