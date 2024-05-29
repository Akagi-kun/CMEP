#include "SceneLoader.hpp"
#include "EventHandling.hpp"
#include "AssetManager.hpp"
#include "Engine.hpp"
#include "Rendering/AxisRenderer.hpp"
#include "Rendering/MeshRenderer.hpp"
#include "Rendering/TextRenderer.hpp"
#include "Rendering/SpriteRenderer.hpp"

#include <fstream>

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_SCENE_LOADER
#include "Logging/LoggingPrefix.hpp"

namespace Engine
{
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
		catch(std::exception& e)
		{
			this->logger->SimpleLog(Logging::LogLevel::Exception, LOGPFX_CURRENT "Error parsing scene.json '%s', what: %s", std::string(this->scene_prefix + "/" + scene_name + "/scene.json").c_str(), e.what());
			throw;
		}

		this->logger->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "Loading scene prefix is: %s", scene_path.c_str());

        std::weak_ptr<AssetManager> asset_manager = this->owner_engine->GetAssetManager();

		this->LoadSceneAssets(data, scene_path);

		if(auto& locked_asset_manager = asset_manager.lock())
		{
			// Load scene event handlers
			for(auto& event_handler_entry : data["eventHandlers"])
			{
				EventHandling::EventType event_type = EventHandling::EventType::EVENT_UNDEFINED;

				std::string event_handler_type = event_handler_entry["type"];
				std::string event_handler_file = event_handler_entry["file"];
				std::string event_handler_function = event_handler_entry["function"];

				const auto& mappedType = EventHandling::eventTypeMap.find(event_handler_type);

				if (mappedType != EventHandling::eventTypeMap.end())
				{
					event_type = mappedType->second;
					this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Event handler for type: %s", event_handler_type.c_str());
				}
				else
				{
					this->logger->SimpleLog(Logging::LogLevel::Warning, LOGPFX_CURRENT "Unknown event type '%s'", event_handler_type.c_str());
					continue;
				}

				//assert(event_type != EventHandling::EventType::EVENT_UNDEFINED);

				std::shared_ptr<Scripting::LuaScript> event_handler = locked_asset_manager->GetLuaScript(event_handler_file);
				
				if(event_handler == nullptr)
				{
					// TODO: Don't add assets with name = location!
					locked_asset_manager->AddLuaScript(scene_path + event_handler_file, scene_path + event_handler_file);
					event_handler = locked_asset_manager->GetLuaScript(scene_path + event_handler_file);
				}
				
				scene->lua_event_handlers.emplace(event_type, std::make_pair(event_handler, event_handler_entry["function"]));
			}

			// Load scene object templates
			for(auto& template_entry : data["templates"])
			{
				Object* object = new Object();

				Rendering::GLFWwindowData window_data = this->owner_engine->GetRenderingEngine()->GetWindow();

				if(template_entry["renderer"]["type"] == std::string("sprite"))
				{
					object->renderer = new Rendering::SpriteRenderer(this->owner_engine);
					((Rendering::SpriteRenderer*)object->renderer)->UpdateTexture(locked_asset_manager->GetTexture(template_entry["renderer"]["sprite"]));
					((Rendering::SpriteRenderer*)object->renderer)->scene_manager = this->owner_engine->GetSceneManager();
					object->renderer_type = "sprite";
				}
				
				this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Loaded templated object %s", std::string(template_entry["name"]).c_str());
				scene->LoadTemplatedObject(template_entry["name"], object);
			}
		}
    }

    void SceneLoader::LoadSceneAssets(nlohmann::json& data, std::string& scene_path)
	{
        std::weak_ptr<AssetManager> asset_manager = this->owner_engine->GetAssetManager();

		if(auto& locked_asset_manager = asset_manager.lock())
		{
			for(auto& asset_entry : data["assets"])
			{
				// Enclose in try/catch
				try 
				{
					std::string asset_type = asset_entry["type"];
					std::string asset_name = asset_entry["name"];
					std::string asset_location = asset_entry["location"];

					if(asset_type == "texture")
					{
						locked_asset_manager->AddTexture(asset_name, scene_path + asset_location, Rendering::Texture_InitFiletype::FILE_PNG);
					}
					else if(asset_type == "script")
					{
						locked_asset_manager->AddLuaScript(asset_name, scene_path + asset_location);
					}
					else if(asset_type == "font")
					{
						// Font assets currently have to list all textures they use
						// this is suboptimal design and might TODO: change in the future?
						for(auto& page_entry : asset_entry["textures"])
						{
							std::string page_name = page_entry["name"];
							std::string page_location = page_entry["location"];

							locked_asset_manager->AddTexture(page_name, scene_path + page_location, Rendering::Texture_InitFiletype::FILE_PNG);
						}
					
						// Once all textures have been loaded, finally load the font
						locked_asset_manager->AddFont(asset_name, scene_path + asset_location);
					}
					else if(asset_type == "model")
					{
						locked_asset_manager->AddModel(asset_name, scene_path + asset_location);
					}
					else
					{
						this->logger->SimpleLog(Logging::LogLevel::Warning, LOGPFX_CURRENT "Unknown type '%s' for asset '%s'", asset_type.c_str(), asset_name.c_str());
						continue;
					}

					this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Loaded asset '%s' as '%s'", asset_name.c_str(), asset_type.c_str());
				}
				catch(std::exception& e)
				{
					this->logger->SimpleLog(Logging::LogLevel::Exception, LOGPFX_CURRENT "Exception when loading assets (check scene.json) e.what(): %s", e.what());
					// TODO: This should not rethrow (safe handling?)
					throw;
				}
			}
		}
	}
}