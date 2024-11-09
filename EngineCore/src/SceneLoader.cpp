#include "SceneLoader.hpp"

#include "Assets/AssetManager.hpp"
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
#include <functional>
#include <memory>
#include <string>

namespace Engine
{
#pragma region Static

	namespace
	{
		template <typename supply_data_t>
		[[nodiscard]] supply_data_t interpretSupplyData(
			AssetRepository&			 asset_repository,
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
						auto asset = asset_repository.getAsset<Rendering::Font>(value);
						EXCEPTION_ASSERT(
							asset.has_value(),
							std::format("Could not find asset '{}'", value)
						);
						return {type, asset.value()};
					}
					case RendererSupplyData::Type::TEXTURE:
					{
						auto asset = asset_repository.getAsset<Rendering::Texture>(value);
						EXCEPTION_ASSERT(
							asset.has_value(),
							std::format("Could not find asset '{}'", value)
						);
						return {type, asset.value()};
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

		/**
		 * @sa loadSceneAssetType()
		 */
		using asset_loader_fn_t = std::function<
			void(Engine*, AssetRepository&, const std::string&, const std::filesystem::path&, const nlohmann::json&)>;

		/**
		 * Load asset of a specific type
		 *
		 * @tparam type            Type of asset
		 * @param engine           Used to initialize factories
		 * @param asset_repository Repository to add the asset to
		 * @param asset_name       Name of asset
		 * @param asset_path       Path to asset
		 * @param json_entry       JSON entry for the asset
		 */
		template <AssetType type>
		void loadSceneAssetType(
			Engine*						 engine,
			AssetRepository&			 asset_repository,
			const std::string&			 asset_name,
			const std::filesystem::path& asset_path,
			const nlohmann::json&		 json_entry
		)
		{
			/**
			 * @todo Make factories take only necessary objects instead of entire engine
			 */
			// Gets constructed on first call to this function
			static auto texture_factory = Factories::TextureFactory(engine);
			static auto font_factory	= Factories::FontFactory(engine);

			if constexpr (type == AssetType::TEXTURE)
			{
				vk::Filter filtering = vk::Filter::eLinear;
				if (json_entry.contains("filtering"))
				{
					filtering =
						EnumStringConvertor<vk::Filter>(json_entry["filtering"].get<std::string>());
				}

				// Check if a specific sampling is requested, otherwise use default
				vk::SamplerAddressMode sampling_mode = vk::SamplerAddressMode::eRepeat;
				if (json_entry.contains("sampling_mode"))
				{
					sampling_mode = EnumStringConvertor<vk::SamplerAddressMode>(
						json_entry["sampling_mode"].get<std::string>()
					);
				}

				asset_repository.addAsset(
					asset_name,
					texture_factory.createTexture(asset_path, filtering, sampling_mode)
				);
			}
			else if constexpr (type == AssetType::FONT)
			{
				// Use a lambda as the callback to createFont
				auto callback = [&](const std::filesystem::path& path) {
					assert(!path.empty());

					return texture_factory.createTexture(path);
				};

				asset_repository.addAsset(asset_name, font_factory.createFont(asset_path, callback));
			}
			else if constexpr (type == AssetType::SCRIPT)
			{
				Scripting::ILuaScript* script;

				if (json_entry.contains("is_generator"))
				{
					script = new Scripting::GeneratorLuaScript(engine, asset_path);
				}
				else { script = new Scripting::EventLuaScript(engine, asset_path); }

				asset_repository.addAsset(asset_name, std::shared_ptr<Scripting::ILuaScript>(script));
			}
		}

		void loadSceneAssetEntry(
			Engine*						 engine,
			AssetRepository&			 asset_repository,
			const nlohmann::json&		 asset_entry,
			const std::filesystem::path& scene_path
		)
		{
			AssetType asset_type =
				EnumStringConvertor<AssetType>(asset_entry["type"].get<std::string>());

			std::string			  asset_name = asset_entry["name"].get<std::string>();
			std::filesystem::path asset_path = scene_path /
											   asset_entry["location"].get<std::string>();

			asset_loader_fn_t loader_fn;

			switch (asset_type)
			{
				case AssetType::TEXTURE:
				{
					loader_fn = loadSceneAssetType<AssetType::TEXTURE>;
					break;
				}
				case AssetType::SCRIPT:
				{
					loader_fn = loadSceneAssetType<AssetType::SCRIPT>;
					break;
				}
				case AssetType::FONT:
				{
					loader_fn = loadSceneAssetType<AssetType::FONT>;
					break;
				}
				default:
					throw ENGINE_EXCEPTION(std::format(
						"Unknown type '{}' for asset '{}'",
						asset_entry["type"].get<std::string>(),
						asset_name
					));
			}

			// Load the asset
			loader_fn(engine, asset_repository, asset_name, asset_path, asset_entry);
		}

	} // namespace

#pragma endregion

#pragma region Public

	SceneLoader::~SceneLoader()
	{
		this->logger->logSingle<decltype(this)>(Logging::LogLevel::VerboseDebug, "Destructor called");
	}

	std::shared_ptr<Scene> SceneLoader::loadScene(const std::string& scene_name)
	{
		std::shared_ptr<Scene> new_scene = std::make_shared<Scene>(owner_engine);

		this->logger
			->logSingle<decltype(this)>(Logging::LogLevel::Info, "Loading scene: '{}'", scene_name);

		loadSceneInternal(new_scene, scene_name);

		return new_scene;
	}

#pragma endregion

#pragma region Protected

	void SceneLoader::loadSceneInternal(std::shared_ptr<Scene>& scene, const std::string& scene_name)
	{
		const std::filesystem::path scene_path = scene_prefix / scene_name;

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

		this->logger->logSingle<decltype(this)>(
			Logging::LogLevel::VerboseDebug,
			"Loading scene prefix is: '{}'",
			scene_path.string()
		);

		try
		{
			loadSceneAssets(data, scene_path, scene);
			loadSceneEventHandlers(data, scene);
			loadSceneTemplates(data, scene);
		}
		catch (...)
		{
			std::throw_with_nested(ENGINE_EXCEPTION("Failed on scene load"));
		}
	}

	void SceneLoader::loadSceneEventHandlers(const nlohmann::json& data, std::shared_ptr<Scene>& scene)
	{
		// Load scene event handlers
		for (const auto& event_handler_entry : data["event_handlers"])
		{
			std::string event_handler_type = event_handler_entry["type"].get<std::string>();
			std::string script_name		   = event_handler_entry["file"].get<std::string>();
			std::string script_function	   = event_handler_entry["function"].get<std::string>();

			EventHandling::EventType event_type =
				EnumStringConvertor<EventHandling::EventType>(event_handler_type);

			auto handler_script =
				scene->asset_repository->getAsset<Scripting::ILuaScript>(script_name);

			EXCEPTION_ASSERT(
				handler_script.has_value(),
				std::format(
					"Asset script '{}' required to serve defined event handlers!",
					script_name
				)
			);

			scene->lua_event_handlers.emplace(
				event_type,
				Scripting::ScriptFunctionRef{
					.script	  = handler_script.value(),
					.function = script_function
				}
			);
		}

		this->logger->logSingle<decltype(this)>(
			Logging::LogLevel::Debug,
			"Done stage: Event Handlers"
		);
	}

	void SceneLoader::loadSceneTemplates(const nlohmann::json& data, std::shared_ptr<Scene>& scene)
	{
		// Load scene object templates
		for (const auto& entry : data["templates"])
		{
			Factories::ObjectFactory::ObjectTemplate obj_template = {
				.with_renderer			 = entry["renderer"].get<std::string>(),
				.with_mesh_builder		 = entry["mesh_builder"].get<std::string>(),
				.with_shader			 = entry["shader_name"].get<std::string>(),
				.renderer_supply_list	 = {},
				.meshbuilder_supply_list = {}
			};
			std::string name = entry["name"].get<std::string>();

			for (const auto& supply_entry : entry["renderer_supply_data"])
			{
				EnumStringConvertor<Rendering::RendererSupplyData::Type> type =
					supply_entry[0].get<std::string>();

				std::string val = supply_entry[1].get<std::string>();

				obj_template.renderer_supply_list.emplace_back(
					interpretSupplyData<Rendering::RendererSupplyData>(
						*scene->asset_repository,
						type,
						val
					)
				);
			}

			for (const auto& supply_entry : entry["meshbuilder_supply_data"])
			{
				EnumStringConvertor<Rendering::MeshBuilderSupplyData::Type> type =
					supply_entry[0].get<std::string>();

				std::string val = supply_entry[1].get<std::string>();

				obj_template.meshbuilder_supply_list.emplace_back(
					interpretSupplyData<Rendering::MeshBuilderSupplyData>(
						*scene->asset_repository,
						type,
						val
					)
				);
			}

			scene->loadTemplatedObject(name, obj_template);

			this->logger->logSingle<decltype(this)>(
				Logging::LogLevel::VerboseDebug,
				"Loaded template '{}'",
				name
			);
		}

		this->logger->logSingle<decltype(this)>(Logging::LogLevel::Debug, "Done stage: Templates");
	}

	void SceneLoader::loadSceneAssets(
		const nlohmann::json&		 data,
		const std::filesystem::path& scene_path,
		std::shared_ptr<Scene>&		 scene
	)
	{
		for (const auto& asset_entry : data["assets"])
		{
			try
			{
				loadSceneAssetEntry(owner_engine, *scene->asset_repository, asset_entry, scene_path);
			}
			catch (...)
			{
				std::throw_with_nested(ENGINE_EXCEPTION(std::format(
					"Exception occured loading asset! Relevant JSON:\n'{}'",
					asset_entry.dump(4)
				)));
			}
		}

		this->logger->logSingle<decltype(this)>(Logging::LogLevel::Debug, "Done stage: Assets");
	}

#pragma endregion

} // namespace Engine
