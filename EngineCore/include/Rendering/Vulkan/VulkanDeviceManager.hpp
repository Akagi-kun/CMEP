#pragma once

#include "ImportVulkan.hpp"
#include "InternalEngineObject.hpp"
#include "VulkanStructDefs.hpp"


namespace Engine::Rendering
{
	class VulkanDeviceManager final : public InternalEngineObject
	{
	private:
		// Defaults
		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

		// Queues
		QueueFamilyIndices graphicsQueueIndices{};
		VkQueue vkGraphicsQueue = VK_NULL_HANDLE;
		VkQueue vkPresentQueue = VK_NULL_HANDLE;

		// GLFW window
		GLFWwindow* window = nullptr;
		unsigned int windowX = 0, windowY = 0;
		std::string windowTitle{};

		// Vulkan devices
		VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
		VkDevice vkLogicalDevice = VK_NULL_HANDLE;

		// Vulkan instance
		VkInstance vkInstance = VK_NULL_HANDLE;

		// Surfaces
		VkSurfaceKHR vkSurface = VK_NULL_HANDLE;

		// Required extensions to be supported
		const std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
			VK_EXT_ROBUSTNESS_2_EXTENSION_NAME,
		};

		// Validation layers
		VkDebugUtilsMessengerEXT vkDebugMessenger = VK_NULL_HANDLE;
#ifndef _DEBUG
		const bool enableVkValidationLayers = false;
#else
		const bool enableVkValidationLayers = true;
#endif

		// Required validation layers to be supported
		const std::vector<const char*> vkValidationLayers = {
			"VK_LAYER_KHRONOS_validation",
		};

		// Internal device functions
		VkSampleCountFlagBits getMaxUsableSampleCount();
		int checkVulkanPhysicalDeviceScore(VkPhysicalDevice device);
		bool checkVulkanDeviceExtensionSupport(VkPhysicalDevice device);
		QueueFamilyIndices findVulkanQueueFamilies(VkPhysicalDevice device);
		SwapChainSupportDetails queryVulkanSwapChainSupport(VkPhysicalDevice device);

		// Internal init functions
		bool checkVulkanValidationLayers();
		void initVulkanInstance();
		void initVulkanDevice();
		void createVulkanLogicalDevice();
		void createVulkanSurface();

	public:
		VulkanDeviceManager();

		void init(GLFWwindow* new_window);

		void cleanup();

		const VkPhysicalDevice& GetPhysicalDevice() const noexcept;
		const VkDevice& GetLogicalDevice() const noexcept;
		const VkInstance& GetInstance() const noexcept;
		const VkSurfaceKHR& GetSurface() const noexcept;
		const VkSampleCountFlagBits& GetMSAASampleCount() const noexcept;
		const QueueFamilyIndices& GetQueueFamilies() const noexcept;
		const VkQueue& GetGraphicsQueue() const noexcept;
		const VkQueue& GetPresentQueue() const noexcept;

		SwapChainSupportDetails QuerySwapChainSupport();
	};
} // namespace Engine::Rendering
