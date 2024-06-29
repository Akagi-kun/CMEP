#pragma once

#include "ImportVulkan.hpp"
#include "VulkanRenderingEngine.hpp"
#include "VulkanStructDefs.hpp"

#include <fstream>
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
		(void)(availablePresentModes);
		// Mailbox potentially worse?
		/*for (const auto& available_present_mode : availablePresentModes)
		{
			if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return available_present_mode;
			}
		}*/

		// Return unpreferred present mode
		// FIFO is guaranteed to be available by the spec
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	inline bool DoesVulkanFormatHaveStencilComponent(VkFormat format) noexcept
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	inline std::vector<char> ReadShaderFile(const std::string& path)
	{
		std::ifstream file(path, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file!");
		}

		size_t file_size = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(file_size);

		file.seekg(0);
		file.read(buffer.data(), static_cast<std::streamsize>(file_size));

		file.close();

		return buffer;
	}

	inline void VulkanUniformBufferTransfer(
		VulkanRenderingEngine* rendering_engine,
		VulkanPipeline* pipeline,
		uint32_t current_frame,
		void* data,
		size_t data_size
	)
	{
		if (auto locked_device_manager = rendering_engine->GetDeviceManager().lock())
		{
			vkMapMemory(
				locked_device_manager->GetLogicalDevice(),
				pipeline->uniform_buffers[current_frame]->allocation_info.deviceMemory,
				pipeline->uniform_buffers[current_frame]->allocation_info.offset,
				pipeline->uniform_buffers[current_frame]->allocation_info.size,
				0,
				&(pipeline->uniform_buffers[current_frame]->mapped_data)
			);

			memcpy(pipeline->uniform_buffers[current_frame]->mapped_data, data, data_size);

			vkUnmapMemory(
				locked_device_manager->GetLogicalDevice(),
				pipeline->uniform_buffers[current_frame]->allocation_info.deviceMemory
			);
		}
	}
} // namespace Engine::Rendering::VulkanUtils
