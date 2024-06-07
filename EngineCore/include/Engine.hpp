#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Rendering/Vulkan/VulkanImageFactory.hpp"
#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"

#include "EventHandling.hpp"
#include "Logging/Logging.hpp"
#include "PlatformSemantics.hpp"
#include "SceneManager.hpp"

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

	extern bool EngineIsWindowInFocus;
	extern bool EngineIsWindowInContent;
	extern double EngineMouseXPos;
	extern double EngineMouseYPos;

	typedef struct structEngineConfig
	{
		struct
		{
			unsigned int sizeX = 0;
			unsigned int sizeY = 0;
			std::string title = "I am a title!";
		} window;

		struct
		{
			unsigned int framerateTarget = 0;
		} rendering;

		struct
		{
			std::string textures;
			std::string models;
			std::string scripts;
			std::string scenes;
		} lookup;

		std::string defaultScene;
	} EngineConfig;

	class Engine final
	{
	private:
		std::string config_path = "";

		// Window
		unsigned int framerateTarget = 30;

		double lastDeltaTime = 0.0;

		std::unique_ptr<EngineConfig> config{};

		// Engine parts
		std::shared_ptr<Logging::Logger> logger{};
		std::shared_ptr<AssetManager> asset_manager{};
		Scripting::LuaScriptExecutor* script_executor = nullptr;
		Rendering::VulkanRenderingEngine* rendering_engine = nullptr;
		std::shared_ptr<Rendering::Factories::VulkanImageFactory> vulkanImageFactory{};

		// Event handler storage
		// std::multimap<EventHandling::EventType, std::function<int(EventHandling::Event&)>> event_handlers;

		static void spinSleep(double seconds);

		static void RenderCallback(VkCommandBuffer commandBuffer, uint32_t currentFrame, Engine* engine);

		static void ErrorCallback(int code, const char* message);
		static void OnWindowFocusCallback(GLFWwindow* window, int focused);
		static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
		static void CursorEnterLeaveCallback(GLFWwindow* window, int entered);
		static void OnKeyEventCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

		void handleInput(const double deltaTime) noexcept;

		void engineLoop();

		void HandleConfig();

	public:
		std::shared_ptr<SceneManager> scene_manager{};

		CMEP_EXPORT Engine(std::shared_ptr<Logging::Logger> logger) noexcept;
		CMEP_EXPORT ~Engine() noexcept;

		CMEP_EXPORT void SetFramerateTarget(unsigned framerate) noexcept;

		CMEP_EXPORT void Init();
		CMEP_EXPORT void Run();

		CMEP_EXPORT void ConfigFile(std::string path);
		// CMEP_EXPORT void RegisterEventHandler(EventHandling::EventType event_type,
		// std::function<int(EventHandling::Event&)> function);

		CMEP_EXPORT void Stop();

		CMEP_EXPORT int FireEvent(EventHandling::Event& event);

		CMEP_EXPORT double GetLastDeltaTime();

		inline std::weak_ptr<AssetManager> GetAssetManager() noexcept
		{
			return this->asset_manager;
		}
		inline Rendering::VulkanRenderingEngine* GetRenderingEngine() noexcept
		{
			return this->rendering_engine;
		}
		inline std::weak_ptr<SceneManager> GetSceneManager() noexcept
		{
			return std::weak_ptr<SceneManager>(this->scene_manager);
		}
		inline std::weak_ptr<Rendering::Factories::VulkanImageFactory> GetVulkanImageFactory() noexcept
		{
			return this->vulkanImageFactory;
		}
	};
} // namespace Engine