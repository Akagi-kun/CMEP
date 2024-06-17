#include "SceneLoader.hpp"

#include "Assets/AssetManager.hpp"
#include "Rendering/AxisRenderer.hpp"
#include "Rendering/IRenderer.hpp"
#include "Rendering/MeshRenderer.hpp"
#include "Rendering/SpriteRenderer.hpp"
#include "Rendering/TextRenderer.hpp"

#include "Engine.hpp"
#include "EventHandling.hpp"
#include "IModule.hpp"

#include <fstream>
#include <stdexcept>

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_SCENE_LOADER
#include "Logging/LoggingPrefix.hpp"

namespace Engine
{
	SceneLoader::SceneLoader(std::shared_ptr<Logging::Logger> with_logger)
	{
		this->logger = with_logger;
	}

	SceneLoader::~SceneLoader()
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Destructor called");
	}

	std::shared_ptr<Scene> SceneLoader::LoadScene(std::string scene_name)
	{
		std::shared_ptr<Scene> new_scene = std::make_shared<Scene>();
		new_scene->UpdateHeldLogger(this->logger);
		new_scene->UpdateOwnerEngine(this->owner_engine);

		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Loading scene: '%s'", scene_name.c_str());

		this->LoadSceneInternal(new_scene, scene_name);

		return new_scene;
	}

	void SceneLoader::LoadSceneInternal(std::shared_ptr<Scene>& scene, std::string& scene_name)
	{
		std::string scene_path = this->scene_prefix + "/" + scene_name + "/";

		std::ifstream file(scene_path + "scene.json");

		nlohmann::json data;
		try
		{
			data = nlohmann::json::parse(file);
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

		file.close();

		this->logger
			->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "Loading scene prefix is: %s", scene_path.c_str());

		// Load Assets
		this->LoadSceneAssets(data, scene_path);

		// Get window config
		const Rendering::GLFWwindowData window_config = this->owner_engine->GetRenderingEngine()->GetWindow();

		// Load Event Handlers
		this->LoadSceneEventHandlers(data, scene);

		// Load Templates
		this->LoadSceneTemplates(data, scene);

		// Load Scene
		this->LoadSceneTree(data, scene);
	}

	RendererType SceneLoader::InterpretRendererType(nlohmann::json& from)
	{
		static const std::map<std::string, RendererType> renderer_type_map = {
			{"text", RendererType::TEXT},
			{"sprite", RendererType::SPRITE},
			{"mesh", RendererType::MESH},
		};

		std::string renderer_type		= from.get<std::string>();
		const auto& found_renderer_type = renderer_type_map.find(renderer_type);

		if (found_renderer_type != renderer_type_map.end())
		{
			return found_renderer_type->second;
		}

		return RendererType::MAX_ENUM;
	}

	void SceneLoader::LoadSceneEventHandlers(nlohmann::json& data, std::shared_ptr<Scene>& scene)
	{
		static const std::map<std::string, EventHandling::EventType> event_type_map = {
			{"onInit", EventHandling::EventType::ON_INIT},
			{"onMouseMoved", EventHandling::EventType::ON_MOUSEMOVED},
			{"onKeyDown", EventHandling::EventType::ON_KEYDOWN},
			{"onKeyUp", EventHandling::EventType::ON_KEYUP},
			{"onUpdate", EventHandling::EventType::ON_UPDATE},
		};

		std::weak_ptr<AssetManager> asset_manager = this->owner_engine->GetAssetManager();
		if (auto locked_asset_manager = asset_manager.lock())
		{
			// Load scene event handlers
			for (auto& event_handler_entry : data["eventHandlers"])
			{
				EventHandling::EventType event_type = EventHandling::EventType::EVENT_UNDEFINED;

				std::string event_handler_type	   = event_handler_entry["type"].get<std::string>();
				std::string event_handler_file	   = event_handler_entry["file"].get<std::string>();
				std::string event_handler_function = event_handler_entry["function"].get<std::string>();

				const auto& mapped_type = event_type_map.find(event_handler_type);

				if (mapped_type != event_type_map.end())
				{
					event_type = mapped_type->second;
					this->logger->SimpleLog(
						Logging::LogLevel::Debug3,
						LOGPFX_CURRENT "Event handler for type: %s",
						event_handler_type.c_str()
					);
				}
				else
				{
					this->logger->SimpleLog(
						Logging::LogLevel::Warning,
						LOGPFX_CURRENT "Unknown event type '%s'",
						event_handler_type.c_str()
					);
					continue;
				}

				std::shared_ptr<Scripting::LuaScript> event_handler = locked_asset_manager->GetLuaScript(
					event_handler_file
				);

				if (event_handler == nullptr)
				{
					throw std::runtime_error(
						"'script' type asset '" + event_handler_file + "' required to serve defined event handlers!"
					);
				}

				scene->lua_event_handlers.emplace(
					event_type,
					std::make_pair(event_handler, event_handler_entry["function"])
				);
			}

			this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Done stage: Event Handlers");
		}
	}

	void SceneLoader::LoadSceneTree(nlohmann::json& data, std::shared_ptr<Scene>& scene)
	{
		// Get window config
		const Rendering::GLFWwindowData window_config = this->owner_engine->GetRenderingEngine()->GetWindow();

		std::weak_ptr<AssetManager> asset_manager = this->owner_engine->GetAssetManager();

		if (auto locked_asset_manager = asset_manager.lock())
		{
			// Load scene
			for (auto& scene_entry : data["scene"])
			{
				std::string object_name = scene_entry["name"].get<std::string>();

				this->logger->SimpleLog(
					Logging::LogLevel::Debug3,
					LOGPFX_CURRENT "Loading scene object '%s'",
					object_name.c_str()
				);

				auto& position_entry = scene_entry["position"];
				glm::vec3 position	 = glm::vec3(
					  position_entry[0].get<float>(),
					  position_entry[1].get<float>(),
					  position_entry[2].get<float>()
				  );

				if (scene_entry["pos_aspixel"].is_array())
				{
					auto& pos_aspixel = scene_entry["pos_aspixel"];

					if (pos_aspixel[0].get<bool>())
					{
						position.x /= static_cast<float>(window_config.window_x);
					}
					if (pos_aspixel[1].get<bool>())
					{
						position.y /= static_cast<float>(window_config.window_y);
					}
				}
				else if (scene_entry["pos_sub_aspixel"].is_array())
				{
					auto& pos_sub_aspixel = scene_entry["pos_sub_aspixel"];

					if (pos_sub_aspixel[0].get<bool>())
					{
						position.x = (static_cast<float>(window_config.window_x) - position.x) /
									 static_cast<float>(window_config.window_x);
					}
					if (pos_sub_aspixel[1].get<bool>())
					{
						position.y = (static_cast<float>(window_config.window_y) - position.y) /
									 static_cast<float>(window_config.window_y);
					}
				}

				auto& scale_entry = scene_entry["scale"];
				glm::vec3 scale	  = glm::vec3(
					  scale_entry[0].get<float>(),
					  scale_entry[1].get<float>(),
					  scale_entry[2].get<float>()
				  );

				if (scene_entry["scale_aspixel"].is_array())
				{
					auto& scale_aspixel = scene_entry["scale_aspixel"];

					if (scale_aspixel[0].get<bool>())
					{
						scale.x /= static_cast<float>(window_config.window_x);
					}

					if (scale_aspixel[1].get<bool>())
					{
						scale.y /= static_cast<float>(window_config.window_y);
					}
				}

				RendererType use_renderer_type = SceneLoader::InterpretRendererType(scene_entry["renderer"]);

				Object* object = nullptr;

				if (RendererType::MIN_ENUM < use_renderer_type && use_renderer_type < RendererType::MAX_ENUM)
				{
					// Allocate new object when renderer type is known
					object = new Object();

					switch (use_renderer_type)
					{
						case RendererType::SPRITE:
						{
							Rendering::IRenderer* with_renderer = new Rendering::SpriteRenderer(this->GetOwnerEngine());

							for (auto& supply_texture : scene_entry["renderer_supply_textures"])
							{
								std::shared_ptr<Rendering::Texture> texture = locked_asset_manager->GetTexture(
									supply_texture.get<std::string>()
								);

								ModuleMessage texture_supply_message = {
									ModuleMessageTarget::RENDERER,
									ModuleMessageType::RENDERER_SUPPLY,
									Rendering::RendererSupplyData{Rendering::RendererSupplyDataType::TEXTURE, texture}
								};

								// with_renderer->SupplyData(texture_supply);
								with_renderer->Communicate(texture_supply_message);
							}

							with_renderer->UpdateMesh();

							auto* old_renderer = object->AssignRenderer(with_renderer);
							assert(old_renderer == nullptr);

							break;
						}
						default:
						{
							delete object; // TODO: Reorder this
							// object.reset();
							break;
						}
					}

					if (object != nullptr)
					{
						object->Translate(position);
						object->Scale(scale);

						scene->AddObject(object_name, object);
					}
				}
				else
				{
					this->logger->SimpleLog(
						Logging::LogLevel::Warning,
						LOGPFX_CURRENT "LoadSceneInternal: Missing or invalid RendererType! (check scene.json)"
					);
					continue;
				}
			}

			this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Done stage: Scene");
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
				ObjectTemplate object = ObjectTemplate();

				RendererType use_renderer_type = SceneLoader::InterpretRendererType(template_entry["renderer"]);

				if (RendererType::MIN_ENUM < use_renderer_type && use_renderer_type < RendererType::MAX_ENUM)
				{
					object.with_renderer = use_renderer_type;
				}
				else
				{
					this->logger->SimpleLog(
						Logging::LogLevel::Warning,
						LOGPFX_CURRENT "LoadSceneTemplates: Missing or invalid RendererType! (check scene.json)"
					);
					continue;
				}

				for (auto& texture_supply_entry : template_entry["renderer_supply_textures"])
				{
					std::shared_ptr<Rendering::Texture> texture = locked_asset_manager->GetTexture(
						texture_supply_entry.get<std::string>()
					);

					Rendering::RendererSupplyData texture_supply(Rendering::RendererSupplyDataType::TEXTURE, texture);
					object.supply_list.insert(object.supply_list.end(), texture_supply);
				}

				std::string name = template_entry["name"].get<std::string>();

				scene->LoadTemplatedObject(name, object);

				this->logger->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "Loaded template '%s'", name.c_str());
			}

			this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Done stage: Templates");
		}
	}

	void SceneLoader::LoadSceneAssets(nlohmann::json& data, std::string& scene_path)
	{
		std::weak_ptr<AssetManager> asset_manager = this->owner_engine->GetAssetManager();

		static const std::map<std::string, VkFilter> filtering_map = {
			{"nearest", VK_FILTER_NEAREST},
			{"linear", VK_FILTER_LINEAR},
		};

		static const std::map<std::string, VkSamplerAddressMode> sampling_map = {
			{"repeat", VK_SAMPLER_ADDRESS_MODE_REPEAT},
			{"clamp", VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE},
			{"clamp_border", VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER},
		};

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
						VkFilter filtering = VK_FILTER_LINEAR;
						// Check if a specific filtering is requested, otherwise use default
						if (asset_entry.contains("filtering"))
						{
							std::string filtering_value = asset_entry["filtering"].get<std::string>();

							const auto& found_filtering = filtering_map.find(filtering_value);

							if (found_filtering != filtering_map.end())
							{
								this->logger->SimpleLog(
									Logging::LogLevel::Debug2,
									LOGPFX_CURRENT "Using filtering '%s'",
									filtering_value.c_str()
								);
								filtering = found_filtering->second;
							}
							else
							{
								this->logger->SimpleLog(
									Logging::LogLevel::Warning,
									LOGPFX_CURRENT "Unknown filtering '%s' (will use default)",
									filtering_value.c_str()
								);
							}
						}

						VkSamplerAddressMode sampling_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
						// Check if a specific sampling is requested, otherwise use default
						if (asset_entry.contains("sampling_mode"))
						{
							std::string sampling_mode_value = asset_entry["sampling_mode"].get<std::string>();

							const auto& found_sampling_mode = sampling_map.find(sampling_mode_value);

							if (found_sampling_mode != sampling_map.end())
							{
								this->logger->SimpleLog(
									Logging::LogLevel::Debug2,
									LOGPFX_CURRENT "Using sampling mode '%s'",
									sampling_mode_value.c_str()
								);
								sampling_mode = found_sampling_mode->second;
							}
							else
							{
								this->logger->SimpleLog(
									Logging::LogLevel::Warning,
									LOGPFX_CURRENT "Unknown sampling mode '%s' (will use default)",
									sampling_mode_value.c_str()
								);
							}
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
