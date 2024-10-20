#pragma once
// IWYU pragma: private; include Rendering/Vulkan/common.hpp

#include "Rendering/Transform.hpp"

#include "Exception.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <vector>

namespace Engine::Rendering::Vulkan::Utility
{
	/**
	 * Converts @ref ImageSize to a @c vk::Extent2D
	 */
	template <typename target_t>
	[[nodiscard]] constexpr auto convertToExtent(const ImageSize& size) -> target_t
		requires(std::is_same_v<target_t, vk::Extent2D>)
	{
		return {static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y)};
	}

	/**
	 * Converts @ref ImageSize to a @c vk::Extent3D
	 */
	template <typename target_t>
	[[nodiscard]] constexpr auto convertToExtent(const ImageSize& size, uint32_t depth) -> target_t
		requires(std::is_same_v<target_t, vk::Extent3D>)
	{
		return {static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y), depth};
	}

	inline vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
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

	inline vk::PresentModeKHR chooseSwapPresentMode(
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
	}

	template <typename val_t = char>
	std::vector<val_t> readShaderFile(const std::filesystem::path& path)
	{
		// Open file in binary mode
		// using "ate" mode, file is seeked to end of file
		std::ifstream file(path, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw ENGINE_EXCEPTION(std::format("failed to open shader file! File: {}", path.string())
			);
		}

		// Get file size
		size_t file_size = static_cast<size_t>(file.tellg());
		file.seekg(0);

		std::vector<val_t> buffer(file_size);

		// Read whole file into buffer
		// reinterpret_cast into char* because read can only accept "char", even in binary mode
		file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(file_size));

		file.close();

		return buffer;
	}

	inline vk::SampleCountFlagBits getMaxFramebufferSampleCount(const vk::raii::PhysicalDevice& device
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
