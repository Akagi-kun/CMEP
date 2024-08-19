#pragma once

#include "ImportVulkan.hpp"
#include "VulkanStructDefs.hpp"

namespace Engine::Rendering::Vulkan
{
	class Instance;

	struct Surface
	{
		const Instance* created_by;
		VkSurfaceKHR native_handle;

		[[nodiscard]] SwapChainSupportDetails QueryVulkanSwapChainSupport(vk::PhysicalDevice device) const;
		bool QueryQueueSupport(vk::PhysicalDevice physical_device, uint32_t queue_family) const;
		[[nodiscard]] QueueFamilyIndices FindVulkanQueueFamilies(vk::PhysicalDevice device) const;
	};
} // namespace Engine::Rendering::Vulkan
