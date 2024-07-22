#pragma once

#include "Rendering/Vulkan/ImportVulkan.hpp"
#include "Rendering/Vulkan/Wrappers/framework.hpp"

#include "InternalEngineObject.hpp"
#include "VulkanStructDefs.hpp"

namespace Engine::Rendering::Vulkan
{
	class VDeviceManager final : public InternalEngineObject
	{
	private:
		// Defaults
		VkSampleCountFlagBits msaa_samples = VK_SAMPLE_COUNT_1_BIT;

		// Queues
		QueueFamilyIndices graphics_queue_indices{};
		VkQueue vk_graphics_queue = VK_NULL_HANDLE;
		VkQueue vk_present_queue  = VK_NULL_HANDLE;

		// GLFW window
		GLFWwindow* window	  = nullptr;
		unsigned int window_x = 0, window_y = 0;
		std::string window_title;

		// Vulkan devices
		VkPhysicalDevice vk_physical_device = VK_NULL_HANDLE;
		VkDevice vk_logical_device			= VK_NULL_HANDLE;

		// Vulkan instance
		VkInstance vk_instance = VK_NULL_HANDLE;

		// Command buffer pool
		VCommandPool* vk_command_pool = nullptr;

		// Vulkan memory allocator
		VmaAllocator vma_allocator;

		// Surfaces
		VkSurfaceKHR vk_surface = VK_NULL_HANDLE;

		// Required extensions to be supported
		const std::vector<const char*> device_extensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
			VK_EXT_ROBUSTNESS_2_EXTENSION_NAME,
		};

		// Validation layers
		VkDebugUtilsMessengerEXT vk_debug_messenger = VK_NULL_HANDLE;
#ifndef _DEBUG
		const bool enable_vk_validation_layers = false;
#else
		const bool enable_vk_validation_layers = true;
#endif

		// Required validation layers to be supported
		const std::vector<const char*> vk_validation_layers = {
			"VK_LAYER_KHRONOS_validation",
		};

		// Internal device functions
		VkSampleCountFlagBits GetMaxUsableSampleCount();
		int16_t CheckVulkanPhysicalDeviceScore(VkPhysicalDevice device);
		bool CheckVulkanDeviceExtensionSupport(VkPhysicalDevice device);
		QueueFamilyIndices FindVulkanQueueFamilies(VkPhysicalDevice device);
		SwapChainSupportDetails QueryVulkanSwapChainSupport(VkPhysicalDevice device);

		// Internal init functions
		bool CheckVulkanValidationLayers();
		void InitVulkanInstance();
		void InitVulkanDevice();
		void InitVMA();
		void CreateVulkanLogicalDevice();

	public:
		VDeviceManager(Engine* with_engine, GLFWwindow* new_window);
		~VDeviceManager();

		[[nodiscard]] const VkPhysicalDevice& GetPhysicalDevice() noexcept
		{
			return this->vk_physical_device;
		}

		[[nodiscard]] const VkDevice& GetLogicalDevice() noexcept
		{
			return this->vk_logical_device;
		}

		[[nodiscard]] const VmaAllocator& GetVmaAllocator() noexcept
		{
			return this->vma_allocator;
		}

		[[nodiscard]] const VkInstance& GetInstance() noexcept
		{
			return this->vk_instance;
		}

		[[nodiscard]] const QueueFamilyIndices& GetQueueFamilies() const noexcept
		{
			return this->graphics_queue_indices;
		}

		[[nodiscard]] const VkSurfaceKHR& GetSurface() const noexcept
		{
			return this->vk_surface;
		}

		[[nodiscard]] const VkSampleCountFlagBits& GetMSAASampleCount() const noexcept
		{
			return this->msaa_samples;
		}

		[[nodiscard]] const VkQueue& GetGraphicsQueue() noexcept
		{
			return this->vk_graphics_queue;
		}

		[[nodiscard]] const VkQueue& GetPresentQueue() noexcept
		{
			return this->vk_present_queue;
		}

		[[nodiscard]] VCommandPool* GetCommandPool() noexcept
		{
			return this->vk_command_pool;
		}

		[[nodiscard]] SwapChainSupportDetails QuerySwapChainSupport()
		{
			return this->QueryVulkanSwapChainSupport(this->vk_physical_device);
		}

		// SwapChainSupportDetails QuerySwapChainSupport();
	};
} // namespace Engine::Rendering::Vulkan
