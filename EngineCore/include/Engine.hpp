#pragma once

#include <unordered_map>
#include <functional>
#include <vector>
#include <memory>

#include "Rendering/VulkanRenderingEngine.hpp"

#include "GlobalSceneManager.hpp"
#include "Logging/Logging.hpp"
#include "EventHandling.hpp"
#include "PlatformSemantics.hpp"

namespace Engine
{
	class AssetManager;
	class ImageObject;
	class Object;
	namespace Rendering { class Window; }

	namespace Scripting
	{
		class LuaScriptExecutor;
		class LuaScript;
	}

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
	};

	extern bool EngineIsWindowInFocus;
	extern bool EngineIsWindowInContent;
	extern double EngineMouseXPos;
	extern double EngineMouseYPos;

	class CMEP_EXPORT Engine final
	{
	private:
		// Keeps internal loop running
		bool run_threads = true;

		std::string config_path = "";

		// Window
		//GLFWwindow* window = nullptr;
		unsigned int windowX = 0, windowY = 0;
		std::string windowTitle;
		unsigned int framerateTarget = 30;

		double lastDeltaTime = 0.0;

		/*
		// Vulcan info
		VkInstance vkInstance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT vkDebugMessenger = VK_NULL_HANDLE;
		VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
		VkDevice vkLogicalDevice = VK_NULL_HANDLE;
		const std::vector<const char*> vkValidationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

#ifndef _DEBUG
		const bool enableVkValidationLayers = false;
#else
		const bool enableVkValidationLayers = true;
#endif
		*/

		// Engine parts
		Rendering::VulkanRenderingEngine* rendering_engine = nullptr;
		AssetManager* asset_manager = nullptr;
		Scripting::LuaScriptExecutor* script_executor = nullptr;

		// Event handler storage
		std::vector<std::pair<EventHandling::EventType, std::function<void(EventHandling::Event&)>>> event_handlers;
		std::vector<std::tuple<EventHandling::EventType, Scripting::LuaScript*, std::string>> lua_event_handlers;
		/*
		// Vulcan physical device functions
		int checkVulkanPhysicalDeviceSuitability(VkPhysicalDevice device);
		QueueFamilyIndices findVulkanQueueFamilies(VkPhysicalDevice device);

		// Vulcan logical device functions
		void createVulkanLogicalDevice();

		// Vulcan init functions
		bool checkVulkanValidationLayers();
		void initVulkanInstance();
		void initVulkanDevice();
		void initVulkan();
		*/
		//static void APIENTRY debugCallbackGL(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) noexcept;
		static void spinSleep(double seconds);

		static void RenderCallback(VkCommandBuffer commandBuffer, uint32_t currentFrame);

		static void OnWindowFocusCallback(GLFWwindow* window, int focused);
		static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
		static void CursorEnterLeaveCallback(GLFWwindow* window, int entered);
		static void Engine::OnKeyEventCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

		void handleInput(const double deltaTime) noexcept;

		void engineLoop();

		void HandleConfig();

	public:
		Engine(const char* windowTitle, const unsigned windowX, const unsigned windowY) noexcept;
		~Engine() noexcept;

		void SetFramerateTarget(unsigned framerate) noexcept;

		void Init();
		void Run();

		void ConfigFile(std::string path);
		void RegisterEventHandler(EventHandling::EventType event_type, std::function<void(EventHandling::Event&)> function);
		void RegisterLuaEventHandler(EventHandling::EventType event_type, Scripting::LuaScript* script, std::string function);
		
		void FireEvent(EventHandling::Event& event);

		double GetLastDeltaTime();

		AssetManager* GetAssetManager() noexcept;
		Rendering::VulkanRenderingEngine* GetRenderingEngine() noexcept;
	};

	CMEP_EXPORT Engine* initializeEngine(const char* windowTitle, const unsigned windowX, const unsigned windowY);

	extern CMEP_EXPORT Engine* global_engine;
}