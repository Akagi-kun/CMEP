#pragma once

#include "fwd.hpp"

#include "Rendering/Transform.hpp"

#include "Logging/Logging.hpp"

#include "ImportVulkan.hpp"
#include "backend/LogicalDevice.hpp"
#include "backend/MemoryAllocator.hpp"
#include "backend/PhysicalDevice.hpp"
#include "common/HandleWrapper.hpp"
#include "rendering/Window.hpp"

#include <vector>

namespace Engine::Rendering::Vulkan
{
	class Instance final : public Logging::SupportsLogging, public HandleWrapper<vk::raii::Instance>
	{
	public:
		struct WindowParams
		{
			const ScreenSize size;
			const std::string& title;
			const std::vector<std::pair<int, int>>& hints;
		};

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

		[[nodiscard]] PhysicalDevice* GetPhysicalDevice()
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
		vk::raii::DebugUtilsMessengerEXT debug_messenger = nullptr;

		vk::raii::Context context;

		PhysicalDevice* physical_device = nullptr;
		LogicalDevice* logical_device	= nullptr;
		Window* window					= nullptr;

		MemoryAllocator* memory_allocator = nullptr;

		CommandPool* command_pool = nullptr;

		vk::SampleCountFlagBits msaa_samples = vk::SampleCountFlagBits::e1;

		static const std::vector<const char*> validation_layers;

		void InitInstance();
		void InitDevice();

		static bool CheckVulkanValidationLayers();
	};
} // namespace Engine::Rendering::Vulkan
