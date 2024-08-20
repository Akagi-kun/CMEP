#pragma once

#include "ImportVulkan.hpp"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <vector>

namespace Engine::Rendering::Vulkan::Utils
{
	inline vk::SurfaceFormatKHR ChooseVulkanSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& available_formats
	)
	{
		for (const auto& available_format : available_formats)
		{
			if (available_format.format == vk::Format::eB8G8R8A8Srgb /* VK_FORMAT_B8G8R8A8_SRGB */ &&
				available_format.colorSpace ==
					vk::ColorSpaceKHR::eSrgbNonlinear /* VK_COLOR_SPACE_SRGB_NONLINEAR_KHR */)
			{
				return available_format;
			}
		}

		// Return first format found
		return available_formats[0];
	}

	inline vk::PresentModeKHR ChooseVulkanSwapPresentMode(const std::vector<vk::PresentModeKHR>& available_present_modes
	)
	{
		(void)(available_present_modes);
		// Mailbox potentially worse?
		// for (const auto& available_present_mode : availablePresentModes)
		//{
		/* if (available_present_mode == vk::PresentModeKHR::eMailbox)
		{
			return available_present_mode;
		} */
		//}

		// FIFO is guaranteed to be available by the spec
		return vk::PresentModeKHR::eFifo;
		// VK_PRESENT_MODE_FIFO_KHR;
		// return vk::PresentModeKHR::eFifoRelaxed;
	}

	inline bool DoesVulkanFormatHaveStencilComponent(VkFormat format) noexcept
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	inline std::vector<char> ReadShaderFile(const std::filesystem::path& path)
	{
		std::ifstream file(path, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("failed to open shader file!");
		}

		size_t file_size = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(file_size);

		file.seekg(0);
		file.read(buffer.data(), static_cast<std::streamsize>(file_size));

		file.close();

		return buffer;
	}
} // namespace Engine::Rendering::Vulkan::Utils
