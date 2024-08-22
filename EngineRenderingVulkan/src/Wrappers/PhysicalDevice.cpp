#include "Wrappers/PhysicalDevice.hpp"

#include "Wrappers/Surface.hpp"

#include <stdexcept>

namespace Engine::Rendering::Vulkan
{
#pragma region Public

	std::string PhysicalDevice::GetDeviceName() const
	{
		return getProperties().deviceName;
	}

	vk::Format PhysicalDevice::FindSupportedFormat(
		const std::vector<vk::Format>& candidates,
		vk::ImageTiling tiling,
		vk::FormatFeatureFlags features
	) const
	{
		for (vk::Format format : candidates)
		{
			vk::FormatProperties fmt_properties = getFormatProperties(format);

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

	std::optional<QueueFamilyIndices> PhysicalDevice::FindVulkanQueueFamilies(const Surface* with_surface) const
	{
		QueueFamilyIndices indices;

		std::vector<vk::QueueFamilyProperties> queue_families = getQueueFamilyProperties();

		bool graphics_found = false;
		bool present_found	= false;

		uint32_t indice = 0;
		for (const auto& queue_family : queue_families)
		{
			if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics)
			{
				indices.graphics_family = indice;
				graphics_found			= true;
			}

			// check surface support
			if (with_surface->QueryQueueSupport(*this, indice))
			{
				indices.present_family = indice;
				present_found		   = true;
			}

			indice++;
		}

		if (graphics_found && present_found)
		{
			return indices;
		}

		return std::nullopt;
	}

#pragma endregion
} // namespace Engine::Rendering::Vulkan
