#pragma once

#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"

#include "Logging/Logging.hpp"

#include "EventHandling.hpp"
#include "SceneManager.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace Engine
{
	class AssetManager;
	class ImageObject;
	class Object;
	namespace Rendering
	{
		class Window;
	}

	namespace Scripting
	{
		class LuaScriptExecutor;
		class LuaScript;
	} // namespace Scripting

	typedef struct structEngineConfig
	{
		struct
		{
			unsigned int size_x = 0;
			unsigned int size_y = 0;
			std::string title	= "unknown";
		} window;

		struct
		{
			unsigned int framerate_target = 0;
		} rendering;

		std::string scene_path	  = "game/scenes/";
		std::string default_scene = "default";
	} EngineConfig;

	class Engine final
	{
	private:
		std::string config_path;

		// Window
		uint_fast16_t framerate_target = 0;

		double last_delta_time = 0.0;

		std::unique_ptr<EngineConfig> config;

		// Engine parts
		std::shared_ptr<Logging::Logger> logger;
		std::shared_ptr<AssetManager> asset_manager;
		Scripting::LuaScriptExecutor* script_executor			   = nullptr;
		Rendering::Vulkan::VulkanRenderingEngine* rendering_engine = nullptr;

		// Event handler storage
		// std::multimap<EventHandling::EventType, std::function<int(EventHandling::Event&)>> event_handlers;

		static void SpinSleep(double seconds);

		static void RenderCallback(VkCommandBuffer commandBuffer, uint32_t currentFrame, Engine* engine);

		static void ErrorCallback(int code, const char* message);
		static void OnWindowFocusCallback(GLFWwindow* window, int focused);
		static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
		static void CursorEnterLeaveCallback(GLFWwindow* window, int entered);
		static void OnKeyEventCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

		void HandleInput(double deltaTime) noexcept;

		void EngineLoop();

		void HandleConfig();

		std::shared_ptr<SceneManager> scene_manager;

	public:
		bool state_is_window_in_focus;
		bool state_is_window_in_content;
		double state_mouse_x_pos;
		double state_mouse_y_pos;

		Engine(std::shared_ptr<Logging::Logger>& logger) noexcept;
		~Engine() noexcept;

		void Init();
		void Run();

		void ConfigFile(std::string path);
		// void RegisterEventHandler(EventHandling::EventType event_type,
		// std::function<int(EventHandling::Event&)> function);

		void Stop();

		[[noreturn]] static void ThrowTest();

		int FireEvent(EventHandling::Event& event);

		void SetFramerateTarget(uint_fast16_t framerate) noexcept
		{
			this->framerate_target = framerate;
		}

		[[nodiscard]] double GetLastDeltaTime() const;

		[[nodiscard]] std::shared_ptr<Logging::Logger> GetLogger()
		{
			return this->logger;
		}
		[[nodiscard]] std::weak_ptr<AssetManager> GetAssetManager() noexcept
		{
			return this->asset_manager;
		}
		[[nodiscard]] Rendering::Vulkan::VulkanRenderingEngine* GetRenderingEngine() noexcept
		{
			return this->rendering_engine;
		}
		[[nodiscard]] std::weak_ptr<SceneManager> GetSceneManager() const noexcept
		{
			return std::weak_ptr<SceneManager>(this->scene_manager);
		}
	};
} // namespace Engine
