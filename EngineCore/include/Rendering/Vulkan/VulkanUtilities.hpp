#pragma once

#include "ImportVulkan.hpp"

#include <vector>

namespace Engine::Rendering::VulkanUtils
{
	inline VkSurfaceFormatKHR chooseVulkanSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		// Return unpreferred suface format
		return availableFormats[0];
	}

	inline VkPresentModeKHR chooseVulkanSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		// Return unpreferred present mode
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	inline bool doesVulkanFormatHaveStencilComponent(VkFormat format) noexcept
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}
}