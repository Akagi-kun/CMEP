#pragma once

#include <unordered_map>
#include <functional>
#include <vector>
#include <memory>

#include "Rendering/VulkanRenderingEngine.hpp"

#include "GlobalSceneManager.hpp"
#include "Logging/Logging.hpp"
#include "EventHandling.hpp"

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

	class __declspec(dllexport) Engine final
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
		static void APIENTRY debugCallbackGL(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) noexcept;
		static void spinSleep(double seconds);

		static void RenderCallback(VkCommandBuffer commandBuffer, uint32_t currentFrame);

		void handleInput(const double deltaTime) noexcept;

		void engineLoop();

		void HandleConfig();

		void FireEvent(EventHandling::Event& event);
	public:
		Engine(const char* windowTitle, const unsigned windowX, const unsigned windowY) noexcept;
		~Engine() noexcept;

		void SetFramerateTarget(unsigned framerate) noexcept;

		void Init();
		void Run();

		void ConfigFile(std::string path);
		void RegisterEventHandler(EventHandling::EventType event_type, std::function<void(EventHandling::Event&)> function);
		void RegisterLuaEventHandler(EventHandling::EventType event_type, Scripting::LuaScript* script, std::string function);

		void AddObject(std::string name, Object* ptr);
		Object* FindObject(std::string name);
		size_t RemoveObject(std::string name) noexcept;
		
		AssetManager* GetAssetManager() noexcept;
		Rendering::VulkanRenderingEngine* GetRenderingEngine() noexcept;
	};

	__declspec(dllexport) Engine* initializeEngine(const char* windowTitle, const unsigned windowX, const unsigned windowY);

	extern __declspec(dllexport) Engine* global_engine;
}