#pragma once

#include "ImportVulkan.hpp"

#include <vector>

namespace Engine::Rendering::VulkanUtils
{
	inline VkSurfaceFormatKHR ChooseVulkanSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& available_format : availableFormats)
		{
			if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
				available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return available_format;
			}
		}

		// Return first format found
		return availableFormats[0];
	}

	inline VkPresentModeKHR ChooseVulkanSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& available_present_mode : availablePresentModes)
		{
			if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				// TODO: Check this value
				return available_present_mode;
			}
		}

		// Return unpreferred present mode
		// FIFO is guaranteed to be available by the spec
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	inline bool DoesVulkanFormatHaveStencilComponent(VkFormat format) noexcept
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}
} // namespace Engine::Rendering::VulkanUtils
