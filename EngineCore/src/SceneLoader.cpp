#include "SceneLoader.hpp"

#include "Assets/AssetManager.hpp"
#include "Rendering/IRenderer.hpp"

#include "Factories/ObjectFactory.hpp"

#include "Engine.hpp"
#include "EnumStringConvertor.hpp"
#include "EventHandling.hpp"
#include "vulkan/vulkan_core.h"

#include <fstream>
#include <stdexcept>
#include <string>

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_SCENE_LOADER
#include "Logging/LoggingPrefix.hpp"

namespace Engine
{
	SceneLoader::~SceneLoader()
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Destructor called");
	}

	std::shared_ptr<Scene> SceneLoader::LoadScene(std::string scene_name)
	{
		std::shared_ptr<Scene> new_scene = std::make_shared<Scene>(this->owner_engine);

		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Loading scene: '%s'", scene_name.c_str());

		this->LoadSceneInternal(new_scene, scene_name);

		return new_scene;
	}

	void SceneLoader::LoadSceneInternal(std::shared_ptr<Scene>& scene, std::string& scene_name)
	{
		std::string scene_path = this->scene_prefix + scene_name + "/";

		nlohmann::json data;
		try
		{
			std::ifstream file(scene_path + "scene.json");

			data = nlohmann::json::parse(file);

			file.close();
		}
		catch (std::exception& e)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Exception,
				LOGPFX_CURRENT "Error parsing scene.json '%s'! e.what(): %s",
				std::string(this->scene_prefix + "/" + scene_name + "/scene.json").c_str(),
				e.what()
			);
			throw;
		}

		this->logger
			->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "Loading scene prefix is: %s", scene_path.c_str());

		// Load Assets
		this->LoadSceneAssets(data, scene_path);

		try
		{
			// Load Event Handlers
			this->LoadSceneEventHandlers(data, scene);

			// Load Templates
			this->LoadSceneTemplates(data, scene);
		}
		catch (std::exception& e)
		{
			this->logger
				->SimpleLog(Logging::LogLevel::Exception, "Failed on post-asset scene load! e.what(): %s", e.what());

			throw;
		}
	}

	void SceneLoader::LoadSceneEventHandlers(nlohmann::json& data, std::shared_ptr<Scene>& scene)
	{
		std::weak_ptr<AssetManager> asset_manager = this->owner_engine->GetAssetManager();
		if (auto locked_asset_manager = asset_manager.lock())
		{
			// Load scene event handlers
			for (const auto& event_handler_entry : data["event_handlers"])
			{
				std::string event_handler_type	   = event_handler_entry["type"].get<std::string>();
				std::string event_handler_file	   = event_handler_entry["file"].get<std::string>();
				std::string event_handler_function = event_handler_entry["function"].get<std::string>();

				EventHandling::EventType event_type = EnumStringConvertor<EventHandling::EventType>(event_handler_type);

				std::shared_ptr<Scripting::LuaScript> event_handler = locked_asset_manager->GetLuaScript(
					event_handler_file
				);

				if (event_handler == nullptr)
				{
					throw std::runtime_error(
						"'script' type asset '" + event_handler_file + "' required to serve defined event handlers!"
					);
				}

				scene->lua_event_handlers.emplace(event_type, std::make_pair(event_handler, event_handler_function));
			}

			this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Done stage: Event Handlers");
		}
	}

	void SceneLoader::LoadSceneTemplates(nlohmann::json& data, std::shared_ptr<Scene>& scene)
	{
		std::weak_ptr<AssetManager> asset_manager = this->owner_engine->GetAssetManager();
		if (auto locked_asset_manager = asset_manager.lock())
		{
			// Load scene object templates
			for (auto& template_entry : data["templates"])
			{
				Factories::ObjectFactory::ObjectTemplate object_template{};

				object_template.with_renderer	  = template_entry["renderer"].get<std::string>();
				object_template.with_mesh_builder = template_entry["mesh_builder"].get<std::string>();
				object_template.with_shader		  = template_entry["shader_name"].get<std::string>();

				for (auto& texture_supply_entry : template_entry["renderer_supply_textures"])
				{
					std::shared_ptr<Rendering::Texture> texture = locked_asset_manager->GetTexture(
						texture_supply_entry.get<std::string>()
					);

					Rendering::RendererSupplyData texture_supply(Rendering::RendererSupplyDataType::TEXTURE, texture);
					object_template.supply_list.insert(object_template.supply_list.end(), texture_supply);
				}

				std::string name = template_entry["name"].get<std::string>();

				scene->LoadTemplatedObject(name, object_template);

				this->logger->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "Loaded template '%s'", name.c_str());
			}

			this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Done stage: Templates");
		}
	}

	void SceneLoader::LoadSceneAssets(nlohmann::json& data, std::string& scene_path)
	{
		std::weak_ptr<AssetManager> asset_manager = this->owner_engine->GetAssetManager();

		if (auto locked_asset_manager = asset_manager.lock())
		{
			for (auto& asset_entry : data["assets"])
			{
				// Enclose in try/catch
				try
				{
					std::string asset_type	   = asset_entry["type"].get<std::string>();
					std::string asset_name	   = asset_entry["name"].get<std::string>();
					std::string asset_location = asset_entry["location"].get<std::string>();

					if (asset_type == "texture")
					{
						// Check if a specific filtering is requested, otherwise use default
						VkFilter filtering = VK_FILTER_LINEAR;
						if (asset_entry.contains("filtering"))
						{
							filtering = EnumStringConvertor<VkFilter>(asset_entry["filtering"].get<std::string>());
						}

						// Check if a specific sampling is requested, otherwise use default
						VkSamplerAddressMode sampling_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
						if (asset_entry.contains("sampling_mode"))
						{
							sampling_mode = EnumStringConvertor<VkSamplerAddressMode>(
								asset_entry["sampling_mode"].get<std::string>()
							);
						}

						locked_asset_manager->AddTexture(
							asset_name,
							scene_path + asset_location,
							Rendering::Texture_InitFiletype::FILE_PNG,
							filtering,
							sampling_mode
						);
					}
					else if (asset_type == "script")
					{
						locked_asset_manager->AddLuaScript(asset_name, scene_path + asset_location);
					}
					else if (asset_type == "font")
					{
						// Font assets currently have to list all textures they use
						// this is suboptimal design and might TODO: change in the future?
						for (auto& page_entry : asset_entry["textures"])
						{
							std::string page_name	  = page_entry["name"].get<std::string>();
							std::string page_location = page_entry["location"].get<std::string>();

							locked_asset_manager->AddTexture(
								page_name,
								scene_path + page_location,
								Rendering::Texture_InitFiletype::FILE_PNG
							);
						}

						// Once all textures have been loaded, finally load the font
						locked_asset_manager->AddFont(asset_name, scene_path + asset_location);
					}
					else if (asset_type == "model")
					{
						locked_asset_manager->AddModel(asset_name, scene_path + asset_location);
					}
					else
					{
						this->logger->SimpleLog(
							Logging::LogLevel::Warning,
							LOGPFX_CURRENT "Unknown type '%s' for asset '%s'",
							asset_type.c_str(),
							asset_name.c_str()
						);
						continue;
					}

					this->logger->SimpleLog(
						Logging::LogLevel::Info,
						LOGPFX_CURRENT "Loaded asset '%s' as '%s'",
						asset_name.c_str(),
						asset_type.c_str()
					);
				}
				catch (std::exception& e)
				{
					this->logger->SimpleLog(
						Logging::LogLevel::Exception,
						LOGPFX_CURRENT "Exception when loading assets (check scene.json) e.what(): %s",
						e.what()
					);
					// TODO: This should not rethrow (safe handling?)
					throw;
				}
			}

			this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Done stage: Assets");
		}
	}
} // namespace Engine
