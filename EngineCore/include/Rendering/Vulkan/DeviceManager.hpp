#pragma once

#include "Rendering/Vulkan/ImportVulkan.hpp"
#include "Rendering/Vulkan/Wrappers/framework.hpp"

#include "InternalEngineObject.hpp"
#include "VulkanStructDefs.hpp"
#include "Wrappers/Window.hpp"

namespace Engine::Rendering::Vulkan
{
	class DeviceManager final : public SupportsLogging
	{
	private:
		// Defaults
		VkSampleCountFlagBits msaa_samples = VK_SAMPLE_COUNT_1_BIT;

		// Queues
		QueueFamilyIndices graphics_queue_indices{};
		VkQueue graphics_queue = VK_NULL_HANDLE;
		VkQueue present_queue  = VK_NULL_HANDLE;

		// GLFW window
		Window* window;

		// Vulkan devices
		VkPhysicalDevice physical_device = VK_NULL_HANDLE;
		VkDevice logical_device			 = VK_NULL_HANDLE;

		// Vulkan instance
		VkInstance instance = VK_NULL_HANDLE;

		// Command buffer pool
		CommandPool* command_pool = nullptr;

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
		DeviceManager(SupportsLogging::logger_t with_logger, Window* new_window);
		~DeviceManager();

		[[nodiscard]] const VkPhysicalDevice& GetPhysicalDevice() noexcept
		{
			return this->physical_device;
		}

		[[nodiscard]] const VkDevice& GetLogicalDevice() noexcept
		{
			return this->logical_device;
		}

		[[nodiscard]] const VmaAllocator& GetVmaAllocator() noexcept
		{
			return this->vma_allocator;
		}

		[[nodiscard]] const VkInstance& GetInstance() noexcept
		{
			return this->instance;
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
			return this->graphics_queue;
		}

		[[nodiscard]] const VkQueue& GetPresentQueue() noexcept
		{
			return this->present_queue;
		}

		[[nodiscard]] CommandPool* GetCommandPool() noexcept
		{
			return this->command_pool;
		}

		[[nodiscard]] SwapChainSupportDetails QuerySwapChainSupport()
		{
			return this->QueryVulkanSwapChainSupport(this->physical_device);
		}
	};
} // namespace Engine::Rendering::Vulkan
