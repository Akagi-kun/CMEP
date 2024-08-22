#pragma once

#include "ImportVulkan.hpp"
#include "VulkanStructDefs.hpp"

namespace Engine::Rendering::Vulkan
{
	class Instance;

	struct Surface
	{
		const Instance* created_by;
		vk::SurfaceKHR native_handle;

		[[nodiscard]] SwapChainSupportDetails QueryVulkanSwapChainSupport(const vk::raii::PhysicalDevice& device) const;
		[[nodiscard]] bool QueryQueueSupport(const vk::raii::PhysicalDevice& physical_device, uint32_t queue_family)
			const;
		//[[nodiscard]] QueueFamilyIndices FindVulkanQueueFamilies(const vk::raii::PhysicalDevice& device) const;
	};
} // namespace Engine::Rendering::Vulkan
