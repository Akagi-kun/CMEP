#include "Wrappers/PhysicalDevice.hpp"

#include "Wrappers/Surface.hpp"

#include <stdexcept>

namespace Engine::Rendering::Vulkan
{
#pragma region Public

	std::string PhysicalDevice::GetDeviceName() const
	{
		return native_handle.getProperties().deviceName;
	}

	vk::Format PhysicalDevice::FindSupportedFormat(
		const std::vector<vk::Format>& candidates,
		vk::ImageTiling tiling,
		vk::FormatFeatureFlags features
	) const
	{
		for (vk::Format format : candidates)
		{
			vk::FormatProperties fmt_properties = native_handle.getFormatProperties(format);

			if (tiling == vk::ImageTiling::eLinear && (fmt_properties.linearTilingFeatures & features) == features)
			{
				return format;
			}

			if (tiling == vk::ImageTiling::eOptimal && (fmt_properties.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		throw std::runtime_error("failed to find supported VkFormat!");
	}

	vk::Format PhysicalDevice::FindSupportedDepthFormat() const
	{
		return FindSupportedFormat(
			{vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
			vk::ImageTiling::eOptimal,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment
		);
	}

	QueueFamilyIndices PhysicalDevice::FindVulkanQueueFamilies(const Surface* with_surface) const
	{
		QueueFamilyIndices indices;

		std::vector<vk::QueueFamilyProperties> queue_families = native_handle.getQueueFamilyProperties();

		uint32_t indice = 0;
		for (const auto& queue_family : queue_families)
		{
			if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics)
			{
				indices.graphics_family = indice;
			}

			if (with_surface->QueryQueueSupport(native_handle, indice))
			{
				indices.present_family = indice;
			}

			indice++;
		}

		return indices;
	}

#pragma endregion
} // namespace Engine::Rendering::Vulkan
