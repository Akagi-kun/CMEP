#include "SceneLoader.hpp"

#include "Assets/AssetManager.hpp"
#include "Assets/Texture.hpp"

#include "Scripting/EventLuaScript.hpp"
#include "Scripting/GeneratorLuaScript.hpp"

#include "Factories/FontFactory.hpp"
#include "Factories/ObjectFactory.hpp"
#include "Factories/TextureFactory.hpp"

#include "Engine.hpp"
#include "EnumStringConvertor.hpp"
#include "EventHandling.hpp"
#include "Exception.hpp"
#include "Logging.hpp"
#include "Scene.hpp"
#include "nlohmann/json.hpp"
#include "vulkan/vulkan_enums.hpp"

#include <cassert>
#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <utility>

namespace Engine
{
#pragma region Static

	namespace
	{
		template <typename supply_data_t>
		[[nodiscard]] supply_data_t InterpretSupplyData(
			AssetManager*				 asset_manager,
			typename supply_data_t::Type type,
			const std::string&			 value
		)
		{
			if constexpr (std::is_same_v<supply_data_t, Rendering::RendererSupplyData>)
			{
				switch (type)
				{
					case Rendering::RendererSupplyData::Type::FONT:
					{
						return {type, asset_manager->GetFont(value)};
					}
					case Rendering::RendererSupplyData::Type::TEXTURE:
					{
						return {type, asset_manager->GetTexture(value)};
					}
					default:
					{
						throw ENGINE_EXCEPTION("Invalid supply data type!");
					}
				}
			}
			else if constexpr (std::is_same_v<supply_data_t, Rendering::MeshBuilderSupplyData>)
			{
				switch (type)
				{
					case Rendering::MeshBuilderSupplyData::Type::TEXT:
					{
						return {type, value};
					}
					// TODO: Generator?
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
		this->logger->SimpleLog<decltype(this)>(
			Logging::LogLevel::VerboseDebug,
			"Destructor called"
		);
	}

	std::shared_ptr<Scene> SceneLoader::LoadScene(std::string scene_name)
	{
		std::shared_ptr<Scene> new_scene = std::make_shared<Scene>(owner_engine);

		this->logger->SimpleLog<decltype(this)>(
			Logging::LogLevel::Info,
			"Loading scene: '%s'",
			scene_name.c_str()
		);

		LoadSceneInternal(new_scene, scene_name);

		return new_scene;
	}

#pragma endregion

#pragma region Protected

	void SceneLoader::LoadSceneInternal(std::shared_ptr<Scene>& scene, std::string& scene_name)
	{
		std::string scene_path = scene_prefix + scene_name + "/";

		nlohmann::json data;
		try
		{
			std::ifstream file(scene_path + "scene.json");

			data = nlohmann::json::parse(file);

			file.close();
		}
		catch (...)
		{
			std::throw_with_nested(ENGINE_EXCEPTION("Failed on json parse"));
		}

		this->logger->SimpleLog<decltype(this)>(
			Logging::LogLevel::VerboseDebug,
			"Loading scene prefix is: %s",
			scene_path.c_str()
		);

		try
		{
			// Load Assets
			LoadSceneAssets(data, scene_path);
		}
		catch (...)
		{
			std::throw_with_nested(ENGINE_EXCEPTION("Caught exception loading assets"));
		}

		try
		{
			// Load Event Handlers
			LoadSceneEventHandlers(data, scene);

			// Load Templates
			LoadSceneTemplates(data, scene);
		}
		catch (...)
		{
			std::throw_with_nested(ENGINE_EXCEPTION("Failed on post-asset scene load"));
		}
	}

	void SceneLoader::LoadSceneEventHandlers(nlohmann::json& data, std::shared_ptr<Scene>& scene)
	{
		std::weak_ptr<AssetManager> asset_manager = owner_engine->GetAssetManager();
		if (auto locked_asset_manager = asset_manager.lock())
		{
			// Load scene event handlers
			for (const auto& event_handler_entry : data["event_handlers"])
			{
				std::string event_handler_type = event_handler_entry["type"].get<std::string>();
				std::string script_name		   = event_handler_entry["file"].get<std::string>();
				std::string script_function	   = event_handler_entry["function"].get<std::string>();

				EventHandling::EventType event_type = EnumStringConvertor<EventHandling::EventType>(
					event_handler_type
				);

				std::shared_ptr<Scripting::ILuaScript> event_handler =
					locked_asset_manager->GetLuaScript(script_name);

				if (event_handler == nullptr)
				{
					throw ENGINE_EXCEPTION(
						"Asset type 'script' script '" + script_name +
						"' required to serve defined event handlers!"
					);
				}

				scene->lua_event_handlers.emplace(
					event_type,
					std::make_pair(event_handler, script_function)
				);
			}

			this->logger->SimpleLog<decltype(this)>(
				Logging::LogLevel::Debug,
				"Done stage: Event Handlers"
			);
		}
	}

	void SceneLoader::LoadSceneTemplates(nlohmann::json& data, std::shared_ptr<Scene>& scene)
	{
		std::weak_ptr<AssetManager> asset_manager = owner_engine->GetAssetManager();
		if (auto locked_asset_manager = asset_manager.lock())
		{
			// Load scene object templates
			for (auto& template_entry : data["templates"])
			{
				Factories::ObjectFactory::ObjectTemplate object_template{};

				object_template.with_renderer	  = template_entry["renderer"].get<std::string>();
				object_template.with_mesh_builder = template_entry["mesh_builder"].get<std::string>(
				);
				object_template.with_shader = template_entry["shader_name"].get<std::string>();

				for (auto& supply_entry : template_entry["renderer_supply_data"])
				{
					EnumStringConvertor<Rendering::RendererSupplyData::Type> type =
						supply_entry[0].get<std::string>();
					std::string val = supply_entry[1].get<std::string>();

					object_template.renderer_supply_list.emplace_back(
						InterpretSupplyData<Rendering::RendererSupplyData>(
							locked_asset_manager.get(),
							type,
							val
						)
					);
				}

				for (auto& supply_entry : template_entry["meshbuilder_supply_data"])
				{
					EnumStringConvertor<Rendering::MeshBuilderSupplyData::Type> type =
						supply_entry[0].get<std::string>();
					std::string val = supply_entry[1].get<std::string>();

					object_template.meshbuilder_supply_list.emplace_back(
						InterpretSupplyData<Rendering::MeshBuilderSupplyData>(
							locked_asset_manager.get(),
							type,
							val
						)
					);
				}

				std::string name = template_entry["name"].get<std::string>();

				scene->LoadTemplatedObject(name, object_template);

				this->logger->SimpleLog<decltype(this)>(
					Logging::LogLevel::VerboseDebug,
					"Loaded template '%s'",
					name.c_str()
				);
			}

			this->logger->SimpleLog<decltype(this)>(
				Logging::LogLevel::Debug,
				"Done stage: Templates"
			);
		}
	}

	void SceneLoader::LoadSceneAsset(
		std::shared_ptr<AssetManager>& asset_manager,
		nlohmann::json&				   asset_entry,
		const std::string&			   scene_path
	)
	{
		// Gets constructed on first call to this function
		static auto font_factory	= Factories::FontFactory(owner_engine);
		static auto texture_factory = Factories::TextureFactory(owner_engine);

		EnumStringConvertor<AssetType> asset_type	  = asset_entry["type"].get<std::string>();
		std::string					   asset_name	  = asset_entry["name"].get<std::string>();
		std::string					   asset_location = asset_entry["location"].get<std::string>();

		if (asset_type == AssetType::TEXTURE)
		{
			// Check if a specific filtering is requested, otherwise use default
			vk::Filter filtering = vk::Filter::eLinear;
			if (asset_entry.contains("filtering"))
			{
				filtering = EnumStringConvertor<vk::Filter>(
					asset_entry["filtering"].get<std::string>()
				);
			}

			// Check if a specific sampling is requested, otherwise use default
			vk::SamplerAddressMode sampling_mode = vk::SamplerAddressMode::eRepeat;
			if (asset_entry.contains("sampling_mode"))
			{
				sampling_mode = EnumStringConvertor<vk::SamplerAddressMode>(
					asset_entry["sampling_mode"].get<std::string>()
				);
			}

			auto texture = texture_factory.InitFile(
				scene_path + asset_location,
				Rendering::Texture_InitFiletype::FILE_PNG,
				filtering,
				sampling_mode
			);

			asset_manager->AddTexture(asset_name, texture);
		}
		else if (asset_type == AssetType::SCRIPT)
		{
			std::shared_ptr<Scripting::ILuaScript> script;

			if (asset_entry.contains("is_generator"))
			{
				script = std::make_shared<Scripting::GeneratorLuaScript>(
					owner_engine,
					scene_path + asset_location
				);
			}
			else
			{
				script = std::make_shared<Scripting::EventLuaScript>(
					owner_engine,
					scene_path + asset_location
				);
			}

			asset_manager->AddLuaScript(asset_name, script);
		}
		else if (asset_type == AssetType::FONT)
		{
			// Use a lambda as the callback to CreateFont
			auto callback = [&](const std::filesystem::path& path) {
				auto texture = asset_manager->GetTexture(path.string());

				if (texture == nullptr)
				{
					assert(!path.empty());

					texture =
						texture_factory.InitFile(path, Rendering::Texture_InitFiletype::FILE_PNG);
				}

				return texture;
			};

			std::shared_ptr<Rendering::Font> font =
				font_factory.CreateFont(scene_path + asset_location, callback);

			asset_manager->AddFont(asset_name, font);
		}
		else
		{
			throw ENGINE_EXCEPTION(std::format(
				"Unknown type '{}' for asset '{}'",
				asset_entry["type"].get<std::string>(),
				asset_name
			));
		}
	}

	void SceneLoader::LoadSceneAssets(nlohmann::json& data, const std::string& scene_path)
	{
		std::weak_ptr<AssetManager> asset_manager = owner_engine->GetAssetManager();

		if (auto locked_asset_manager = asset_manager.lock())
		{
			for (auto& asset_entry : data["assets"])
			{
				try
				{
					LoadSceneAsset(locked_asset_manager, asset_entry, scene_path);
				}
				catch (...)
				{
					// TODO: Potentially handle safely?
					std::throw_with_nested(ENGINE_EXCEPTION(std::format(
						"Exception occured loading asset! Relevant JSON:\n{}",
						asset_entry.dump()
					)));
				}
			}

			this->logger->SimpleLog<decltype(this)>(Logging::LogLevel::Debug, "Done stage: Assets");
		}
	}

#pragma endregion

} // namespace Engine
