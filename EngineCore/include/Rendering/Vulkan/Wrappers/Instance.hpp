#pragma once

#include "HandleWrapper.hpp"
#include "InternalEngineObject.hpp"
#include "LogicalDevice.hpp"
#include "MemoryAllocator.hpp"
#include "PhysicalDevice.hpp"
#include "Window.hpp"
#include "framework.hpp"
#include "vulkan/vulkan_core.h"

#include <vector>

namespace Engine::Rendering::Vulkan
{
	class Instance final : public SupportsLogging, public HandleWrapper<VkInstance>
	{
	public:
		struct WindowParams
		{
			const ScreenSize size;
			const std::string& title;
			const std::vector<std::pair<int, int>>& hints;
		};

		DeviceManager* device_manager;

		Instance(SupportsLogging::logger_t with_logger, const WindowParams&& with_window_parameters);
		~Instance();

		[[nodiscard]] Window* GetWindow()
		{
			return window;
		}

		[[nodiscard]] VkSampleCountFlagBits GetMSAASamples() const
		{
			return msaa_samples;
		}

		[[nodiscard]] PhysicalDevice GetPhysicalDevice()
		{
			return physical_device;
		}

		[[nodiscard]] LogicalDevice* GetLogicalDevice()
		{
			return logical_device;
		}

		[[nodiscard]] CommandPool* GetCommandPool()
		{
			return command_pool;
		}

		[[nodiscard]] MemoryAllocator* GetGraphicMemoryAllocator()
		{
			return memory_allocator;
		}

	private:
		VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;

		PhysicalDevice physical_device;
		LogicalDevice* logical_device = nullptr;
		Window* window				  = nullptr;

		MemoryAllocator* memory_allocator = nullptr;

		CommandPool* command_pool = nullptr;

		VkSampleCountFlagBits msaa_samples = VK_SAMPLE_COUNT_1_BIT;

		static const std::vector<const char*> validation_layers;

		void InitInstance();
		void InitDevice();

		static bool CheckVulkanValidationLayers();
		static VkSampleCountFlagBits GetMaxUsableSampleCount(VkPhysicalDevice device);
	};
} // namespace Engine::Rendering::Vulkan
