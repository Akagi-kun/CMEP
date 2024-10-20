#pragma once

#include "Rendering/Transform.hpp"
#include "Rendering/Vulkan/exports.hpp"

#include "Logging/Logging.hpp"

#include "EventHandling.hpp"
#include "SceneManager.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace Engine
{
	class AssetManager;
	class SceneObject;

	namespace Rendering::Vulkan
	{
		class PipelineManager;
	}

	namespace Scripting
	{
		class ILuaScript;
	}

	struct EngineConfig
	{
		struct
		{
			Rendering::ScreenSize size;
			std::string			  title = "unknown";
		} window;

		uint_fast16_t framerate_target = 0;

		std::string game_path	= "game/";
		std::string scene_path	= "scenes/";
		std::string shader_path = "shaders/";

		std::string default_scene = "default";
	};

	class Engine final
	{
	public:
		Engine(std::shared_ptr<Logging::Logger>& logger);
		~Engine();

		void run();

		void configFile(std::string path);

		void stop();

		/**
		 * Function that throws an exception when called.
		 *
		 * Used to verify that exceptions thrown by the library
		 * can be caught outside it.
		 */
		[[noreturn]] static void throwTest();

		/**
		 * Fires an event, calling every handler registered for that type
		 *
		 * @param event The event object
		 * @return Sum of all exit codes from each handler
		 */
		[[nodiscard]] int fireEvent(EventHandling::Event event);

		void setFramerateTarget(uint_fast16_t framerate)
		{
			config.framerate_target = framerate;
		}

		[[nodiscard]] const std::string& getShaderPath() const
		{
			return config.shader_path;
		}

		[[nodiscard]] double getLastDeltaTime() const
		{
			return last_delta_time;
		}

		[[nodiscard]] std::shared_ptr<Logging::Logger> getLogger() const
		{
			return logger;
		}
		[[nodiscard]] std::weak_ptr<AssetManager> getAssetManager()
		{
			return asset_manager;
		}
		[[nodiscard]] std::shared_ptr<Rendering::Vulkan::PipelineManager> getVulkanPipelineManager()
		{
			return pipeline_manager;
		}
		[[nodiscard]] Rendering::Vulkan::Instance* getVulkanInstance()
		{
			return vk_instance;
		}
		[[nodiscard]] std::weak_ptr<SceneManager> getSceneManager()
		{
			return scene_manager;
		}

	private:
		std::string config_path;

		double last_delta_time = 0.0;

		EngineConfig config;

		// Engine parts
		std::shared_ptr<Logging::Logger> logger;
		std::shared_ptr<AssetManager>	 asset_manager;

		Rendering::Vulkan::Instance* vk_instance = nullptr;

		std::shared_ptr<Rendering::Vulkan::PipelineManager> pipeline_manager;

		static void renderCallback(
			Rendering::Vulkan::CommandBuffer* command_buffer,
			uint32_t						  current_frame,
			void*							  engine
		);

		void handleInput(double delta_time);

		void engineLoop();

		void handleConfig();

		std::shared_ptr<SceneManager> scene_manager;
	};
} // namespace Engine
