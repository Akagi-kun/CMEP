#pragma once

#include "Rendering/Transform.hpp"

#include "vulkan/vulkan_raii.hpp"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <vector>

namespace Engine::Rendering::Vulkan::Utility
{
	template <typename T>
	[[nodiscard]] constexpr auto ConvertToExtent(ImageSize& size) -> T
		requires(std::is_same_v<T, vk::Extent2D>)
	{
		return {static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y)};
	}

	template <typename T>
	[[nodiscard]] constexpr auto ConvertToExtent(ImageSize& size, uint32_t depth) -> T
		requires(std::is_same_v<T, vk::Extent3D>)
	{
		return {static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y), depth};
	}

	inline vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(
		const std::vector<vk::SurfaceFormatKHR>& available_formats
	)
	{
		for (const auto& available_format : available_formats)
		{
			if (available_format.format == vk::Format::eB8G8R8A8Srgb &&
				available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				return available_format;
			}
		}

		// Return first format found
		return available_formats[0];
	}

	inline vk::PresentModeKHR ChooseSwapPresentMode(
		const std::vector<vk::PresentModeKHR>& available_present_modes
	)
	{
		(void)(available_present_modes);
		// Mailbox potentially worse?
		/* for (const auto& available_present_mode : available_present_modes)
		{
			if (available_present_mode == vk::PresentModeKHR::eMailbox)
			{
				return available_present_mode;
			}
		}
		*/

		// FIFO is guaranteed to be available by the spec
		return vk::PresentModeKHR::eFifo;
		// return vk::PresentModeKHR::eFifoRelaxed;
	}

	inline std::vector<char> ReadShaderFile(const std::filesystem::path& path)
	{
		std::ifstream file(path, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("failed to open shader file!");
		}

		size_t			  file_size = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(file_size);

		file.seekg(0);
		file.read(buffer.data(), static_cast<std::streamsize>(file_size));

		file.close();

		return buffer;
	}

	inline vk::SampleCountFlagBits GetMaxFramebufferSampleCount(
		const vk::raii::PhysicalDevice& device
	)
	{
		const vk::PhysicalDeviceProperties physical_device_properties = device.getProperties();

		// Check which sample counts are supported by the framebuffers
		vk::SampleCountFlags counts =
			physical_device_properties.limits.framebufferColorSampleCounts &
			physical_device_properties.limits.framebufferDepthSampleCounts;

		if (!counts)
		{
			return vk::SampleCountFlagBits::e1;
		}

		if (counts & vk::SampleCountFlagBits::e64)
		{
			return vk::SampleCountFlagBits::e64;
		}
		if (counts & vk::SampleCountFlagBits::e32)
		{
			return vk::SampleCountFlagBits::e32;
		}
		if (counts & vk::SampleCountFlagBits::e16)
		{
			return vk::SampleCountFlagBits::e16;
		}
		if (counts & vk::SampleCountFlagBits::e8)
		{
			return vk::SampleCountFlagBits::e8;
		}
		if (counts & vk::SampleCountFlagBits::e4)
		{
			return vk::SampleCountFlagBits::e4;
		}
		if (counts & vk::SampleCountFlagBits::e2)
		{
			return vk::SampleCountFlagBits::e2;
		}

		return vk::SampleCountFlagBits::e1;
	}
} // namespace Engine::Rendering::Vulkan::Utility
