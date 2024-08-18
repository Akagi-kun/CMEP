#pragma once

#include "Rendering/Vulkan/VulkanStructDefs.hpp"
#include "Rendering/Vulkan/Wrappers/InstanceOwned.hpp"
#include "Rendering/Vulkan/Wrappers/Queue.hpp"

#include "HandleWrapper.hpp"
#include "vulkan/vulkan_core.h"

namespace Engine::Rendering::Vulkan
{
	class LogicalDevice : public InstanceOwned, public HandleWrapper<VkDevice>
	{
	public:
		LogicalDevice(LogicalDevice& other) = delete;

		LogicalDevice(InstanceOwned::value_t with_instance, const Surface* with_surface);
		~LogicalDevice();

		[[nodiscard]] const QueueFamilyIndices& GetQueueFamilies() const
		{
			return queue_family_indices;
		}

		[[nodiscard]] const Queue& GetGraphicsQueue()
		{
			return graphics_queue;
		}

		[[nodiscard]] const Queue& GetPresentQueue()
		{
			return present_queue;
		}

		void WaitDeviceIdle() const
		{
			vkDeviceWaitIdle(native_handle);
		}

	private:
		QueueFamilyIndices queue_family_indices{};
		Queue graphics_queue = VK_NULL_HANDLE;
		Queue present_queue	 = VK_NULL_HANDLE;

		static const std::vector<const char*> vk_validation_layers;

		[[nodiscard]] Queue GetDeviceQueue(uint32_t family, uint32_t index) const;
	};
} // namespace Engine::Rendering::Vulkan
