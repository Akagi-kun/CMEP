#pragma once

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
	class Object;

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
			std::string title = "unknown";
		} window;

		unsigned int framerate_target = 0;

		std::string scene_path	  = "game/scenes/";
		std::string default_scene = "default";

		std::string shader_path = "game/shaders/vulkan/";
	};

	class Engine final
	{
	private:
		std::string config_path;

		double last_delta_time = 0.0;

		std::unique_ptr<EngineConfig> config;

		// Engine parts
		std::shared_ptr<Logging::Logger> logger;
		std::shared_ptr<AssetManager> asset_manager;

		Rendering::Vulkan::Instance* vk_instance = nullptr;

		std::shared_ptr<Rendering::Vulkan::PipelineManager> pipeline_manager;

		static void SpinSleep(double seconds);

		static void RenderCallback(
			Rendering::Vulkan::CommandBuffer* command_buffer,
			uint32_t current_frame,
			void* engine
		);

		static void ErrorCallback(int code, const char* message);

		void HandleInput(double delta_time);

		void EngineLoop();

		void HandleConfig();

		std::shared_ptr<SceneManager> scene_manager;

	public:
		Engine(std::shared_ptr<Logging::Logger>& logger);
		~Engine();

		void Init();
		void Run();

		void ConfigFile(std::string path);

		void Stop();

		[[noreturn]] static void ThrowTest();

		int FireEvent(EventHandling::Event& event);

		void SetFramerateTarget(uint_fast16_t framerate) noexcept
		{
			config->framerate_target = framerate;
		}

		[[nodiscard]] const std::string& GetShaderPath() const
		{
			return config->shader_path;
		}

		[[nodiscard]] double GetLastDeltaTime() const;

		[[nodiscard]] std::shared_ptr<Logging::Logger> GetLogger() const noexcept
		{
			return logger;
		}
		[[nodiscard]] std::weak_ptr<AssetManager> GetAssetManager() noexcept
		{
			return asset_manager;
		}
		[[nodiscard]] std::shared_ptr<Rendering::Vulkan::PipelineManager> GetVulkanPipelineManager()
		{
			return pipeline_manager;
		}
		[[nodiscard]] Rendering::Vulkan::Instance* GetVulkanInstance()
		{
			return vk_instance;
		}
		[[nodiscard]] std::weak_ptr<SceneManager> GetSceneManager() noexcept
		{
			return scene_manager;
		}
	};
} // namespace Engine
