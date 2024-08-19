#pragma once

#include "ImportVulkan.hpp"
#include "VulkanStructDefs.hpp"
#include "Wrappers/HandleWrapper.hpp"
#include "Wrappers/Surface.hpp"

#include <string>
#include <vector>

namespace Engine::Rendering::Vulkan
{
	class PhysicalDevice final : public HandleWrapper<vk::PhysicalDevice, true>
	{
	public:
		PhysicalDevice() = default;
		PhysicalDevice(vk::PhysicalDevice from_device) : HandleWrapper<vk::PhysicalDevice, true>(from_device)
		{
		}

		[[nodiscard]] std::string GetDeviceName() const;

		[[nodiscard]] VkFormat FindSupportedFormat(
			const std::vector<VkFormat>& candidates,
			VkImageTiling tiling,
			VkFormatFeatureFlags features
		) const;

		[[nodiscard]] VkFormat FindSupportedDepthFormat() const;

		[[nodiscard]] QueueFamilyIndices FindVulkanQueueFamilies(const Surface* with_surface) const;
	};

} // namespace Engine::Rendering::Vulkan
