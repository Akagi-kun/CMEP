#include "SceneLoader.hpp"
#include "AssetManager.hpp"
#include "Engine.hpp"
#include "EventHandling.hpp"
#include "Rendering/AxisRenderer.hpp"
#include "Rendering/MeshRenderer.hpp"
#include "Rendering/SpriteRenderer.hpp"
#include "Rendering/TextRenderer.hpp"

#include <fstream>

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_SCENE_LOADER
#include "Logging/LoggingPrefix.hpp"

namespace Engine
{
	namespace EventHandling
	{
		const std::map<std::string, EventType> eventTypeMap = {
			{"onInit", EventHandling::EventType::ON_INIT},
			{"onMouseMoved", EventHandling::EventType::ON_MOUSEMOVED},
			{"onKeyDown", EventHandling::EventType::ON_KEYDOWN},
			{"onKeyUp", EventHandling::EventType::ON_KEYUP},
			{"onUpdate", EventHandling::EventType::ON_UPDATE},
		};
	}

	SceneLoader::SceneLoader(std::shared_ptr<Logging::Logger> logger)
	{
		this->logger = logger;
	}

	SceneLoader::~SceneLoader()
	{
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
				LOGPFX_CURRENT "Error parsing scene.json '%s', what: %s",
				std::string(this->scene_prefix + "/" + scene_name + "/scene.json").c_str(),
				e.what()
			);
			throw;
		}

		this->logger->SimpleLog(
			Logging::LogLevel::Debug1, LOGPFX_CURRENT "Loading scene prefix is: %s", scene_path.c_str()
		);

		std::weak_ptr<AssetManager> asset_manager = this->owner_engine->GetAssetManager();

		this->LoadSceneAssets(data, scene_path);

		// Get window config
		const Rendering::GLFWwindowData windowConfig = this->owner_engine->GetRenderingEngine()->GetWindow();

		if (auto locked_asset_manager = asset_manager.lock())
		{
			// Load scene event handlers
			for (auto& event_handler_entry : data["eventHandlers"])
			{
				EventHandling::EventType event_type = EventHandling::EventType::EVENT_UNDEFINED;

				std::string event_handler_type = event_handler_entry["type"].get<std::string>();
				std::string event_handler_file = event_handler_entry["file"].get<std::string>();
				std::string event_handler_function = event_handler_entry["function"].get<std::string>();

				const auto& mappedType = EventHandling::eventTypeMap.find(event_handler_type);

				if (mappedType != EventHandling::eventTypeMap.end())
				{
					event_type = mappedType->second;
					this->logger->SimpleLog(
						Logging::LogLevel::Debug3,
						LOGPFX_CURRENT "Event handler for type: %s",
						event_handler_type.c_str()
					);
				}
				else
				{
					this->logger->SimpleLog(
						Logging::LogLevel::Warning, LOGPFX_CURRENT "Unknown event type '%s'", event_handler_type.c_str()
					);
					continue;
				}

				// assert(event_type != EventHandling::EventType::EVENT_UNDEFINED);

				std::shared_ptr<Scripting::LuaScript> event_handler = locked_asset_manager->GetLuaScript(
					event_handler_file
				);

				if (event_handler == nullptr)
				{
					// TODO: Don't add assets with name = location!
					locked_asset_manager->AddLuaScript(
						scene_path + event_handler_file, scene_path + event_handler_file
					);
					event_handler = locked_asset_manager->GetLuaScript(scene_path + event_handler_file);
				}

				scene->lua_event_handlers.emplace(
					event_type, std::make_pair(event_handler, event_handler_entry["function"])
				);
			}

			this->LoadSceneTemplates(data, scene);

			// Load scene
			for (auto& scene_entry : data["scene"])
			{
				std::string object_name = scene_entry["name"].get<std::string>();

				auto& position_entry = scene_entry["position"];
				glm::vec3 position = glm::vec3(
					position_entry[0].get<float>(), position_entry[1].get<float>(), position_entry[2].get<float>()
				);

				if (scene_entry["pos_aspixel"].is_array())
				{
					auto& pos_aspixel = scene_entry["pos_aspixel"];

					if (pos_aspixel[0].get<bool>())
					{
						position.x /= windowConfig.windowX;
					}
					if (pos_aspixel[1].get<bool>())
					{
						position.y /= windowConfig.windowY;
					}
				}
				else if (scene_entry["pos_sub_aspixel"].is_array())
				{
					auto& pos_sub_aspixel = scene_entry["pos_sub_aspixel"];

					if (pos_sub_aspixel[0].get<bool>())
					{
						position.x = (windowConfig.windowX - position.x) / windowConfig.windowX;
					}
					if (pos_sub_aspixel[1].get<bool>())
					{
						position.y = (windowConfig.windowY - position.y) / windowConfig.windowY;
					}
				}

				auto& scale_entry = scene_entry["scale"];
				glm::vec3 scale = glm::vec3(
					scale_entry[0].get<float>(), scale_entry[1].get<float>(), scale_entry[2].get<float>()
				);

				if (scene_entry["scale_aspixel"].is_array())
				{
					auto& scale_aspixel = scene_entry["scale_aspixel"];

					if (scale_aspixel[0].get<bool>())
					{
						scale.x /= windowConfig.windowX;
					}

					if (scale_aspixel[1].get<bool>())
					{
						scale.y /= windowConfig.windowY;
					}
				}

				RendererType useRendererType = this->InterpretRendererType(scene_entry["renderer"]);

				Object* object = nullptr;

				if (RendererType::MIN_ENUM < useRendererType && useRendererType < RendererType::MAX_ENUM)
				{
					// Allocate new object when renderer type is known
					object = new Object();

					switch (useRendererType)
					{
						case RendererType::SPRITE:
						{
							object->renderer = new Rendering::SpriteRenderer(this->GetOwnerEngine());

							for (auto& supplyTexture : scene_entry["renderer_supply_textures"])
							{
								std::shared_ptr<Rendering::Texture> texture = locked_asset_manager->GetTexture(
									supplyTexture.get<std::string>()
								);

								Rendering::RendererSupplyData texture_supply(
									Rendering::RendererSupplyDataType::TEXTURE, texture
								);
								object->renderer->SupplyData(texture_supply);
							}

							object->renderer->UpdateMesh();

							break;
						}
						default:
						{
							delete object; // TODO: Reorder this
							break;
						}
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

				object->Translate(position);
				object->Scale(scale);

				scene->AddObject(object_name, object);
			}
		}
	}

	RendererType SceneLoader::InterpretRendererType(nlohmann::json& from)
	{
		static const std::map<std::string, RendererType> rendererTypeMap = {
			{"text", RendererType::TEXT},
			{"sprite", RendererType::SPRITE},
			{"mesh", RendererType::MESH},
		};

		std::string renderer_type = from.get<std::string>();
		const auto& found_renderer_type = rendererTypeMap.find(renderer_type);

		if (found_renderer_type != rendererTypeMap.end())
		{
			return found_renderer_type->second;
		}
		else
		{
			return RendererType::MAX_ENUM;
		}
	}

	void SceneLoader::LoadSceneTemplates(nlohmann::json& data, std::shared_ptr<Scene>& scene)
	{
		if (auto& locked_asset_manager = this->GetOwnerEngine()->GetAssetManager().lock())
		{
			// Load scene object templates
			for (auto& template_entry : data["templates"])
			{
				ObjectTemplate object = ObjectTemplate();

				RendererType useRendererType = this->InterpretRendererType(template_entry["renderer"]);

				if (RendererType::MIN_ENUM < useRendererType && useRendererType < RendererType::MAX_ENUM)
				{
					object.withRenderer = useRendererType;
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
					std::shared_ptr<Rendering::Texture> texture = locked_asset_manager->GetTexture(texture_supply_entry.get<std::string>());

					Rendering::RendererSupplyData texture_supply(Rendering::RendererSupplyDataType::TEXTURE, texture);
					object.supplyList.insert(object.supplyList.end(), texture_supply);
				}

				std::string name = template_entry["name"].get<std::string>();

				scene->LoadTemplatedObject(name, object);
			}
		}
	}

	void SceneLoader::LoadSceneAssets(nlohmann::json& data, std::string& scene_path)
	{
		std::weak_ptr<AssetManager> asset_manager = this->owner_engine->GetAssetManager();

		static const std::map<std::string, VkFilter> filteringMap = {
			{"nearest", VK_FILTER_NEAREST},
			{"linear", VK_FILTER_LINEAR},
		};

		static const std::map<std::string, VkSamplerAddressMode> samplingMap = {
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
					std::string asset_type = asset_entry["type"].get<std::string>();
					std::string asset_name = asset_entry["name"].get<std::string>();
					std::string asset_location = asset_entry["location"].get<std::string>();

					if (asset_type == "texture")
					{
						VkFilter filtering = VK_FILTER_LINEAR;
						// Check if a specific filtering is requested, otherwise use default
						if (asset_entry.contains("filtering"))
						{
							std::string filtering_value = asset_entry["filtering"].get<std::string>();

							const auto& found_filtering = filteringMap.find(filtering_value);

							if (found_filtering != filteringMap.end())
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

							const auto& found_sampling_mode = samplingMap.find(sampling_mode_value);

							if (found_sampling_mode != samplingMap.end())
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
							std::string page_name = page_entry["name"].get<std::string>();
							std::string page_location = page_entry["location"].get<std::string>();

							locked_asset_manager->AddTexture(
								page_name, scene_path + page_location, Rendering::Texture_InitFiletype::FILE_PNG
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
		}
	}
} // namespace Engine