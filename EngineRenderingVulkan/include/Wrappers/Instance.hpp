#pragma once

#include "Rendering/Transform.hpp"

#include "Logging/Logging.hpp"

#include "ImportVulkan.hpp"
#include "Wrappers/HandleWrapper.hpp"
#include "Wrappers/LogicalDevice.hpp"
#include "Wrappers/MemoryAllocator.hpp"
#include "Wrappers/PhysicalDevice.hpp"
#include "Wrappers/Window.hpp"
#include "framework.hpp"

#include <vector>

namespace Engine::Rendering::Vulkan
{
	class Instance final : public Logging::SupportsLogging, public HandleWrapper<vk::Instance>
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

		[[nodiscard]] vk::SampleCountFlagBits GetMSAASamples() const
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
		vk::DebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;

		PhysicalDevice physical_device;
		LogicalDevice* logical_device = nullptr;
		Window* window				  = nullptr;

		MemoryAllocator* memory_allocator = nullptr;

		CommandPool* command_pool = nullptr;

		vk::SampleCountFlagBits msaa_samples = vk::SampleCountFlagBits::e1;

		static const std::vector<const char*> validation_layers;

		void InitInstance();
		void InitDevice();

		static bool CheckVulkanValidationLayers();
		static vk::SampleCountFlagBits GetMaxUsableSampleCount(vk::PhysicalDevice device);
	};
} // namespace Engine::Rendering::Vulkan
