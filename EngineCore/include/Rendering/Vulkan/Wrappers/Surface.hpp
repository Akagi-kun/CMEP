#pragma once

#include "Rendering/Vulkan/VulkanStructDefs.hpp"

#include "vulkan/vulkan_core.h"

namespace Engine::Rendering::Vulkan
{
	class Instance;

	struct Surface
	{
		const Instance* created_by;
		VkSurfaceKHR native_handle;

		SwapChainSupportDetails QueryVulkanSwapChainSupport(VkPhysicalDevice device) const;
		bool QueryQueueSupport(VkPhysicalDevice physical_device, uint32_t queue_family) const;
		QueueFamilyIndices FindVulkanQueueFamilies(VkPhysicalDevice device) const;
	};
} // namespace Engine::Rendering::Vulkan
