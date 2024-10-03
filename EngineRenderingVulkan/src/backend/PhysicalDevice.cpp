#include "backend/PhysicalDevice.hpp"

#include "Exception.hpp"
#include "common/Utility.hpp"
#include "rendering/Surface.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace Engine::Rendering::Vulkan
{
#pragma region Public

	vk::PhysicalDeviceLimits PhysicalDevice::getLimits() const
	{
		return getProperties().limits;
	}

	vk::SampleCountFlagBits PhysicalDevice::getMSAASamples() const
	{
		return Utility::getMaxFramebufferSampleCount(*this);
	}

	std::string PhysicalDevice::getDeviceName() const
	{
		return getProperties().deviceName;
	}

	vk::Format PhysicalDevice::findSupportedFormat(
		const std::vector<vk::Format>& candidates,
		vk::ImageTiling				   tiling,
		vk::FormatFeatureFlags		   features
	) const
	{
		for (vk::Format format : candidates)
		{
			vk::FormatProperties fmt_properties = getFormatProperties(format);

			if (tiling == vk::ImageTiling::eLinear &&
				(fmt_properties.linearTilingFeatures & features) == features)
			{
				return format;
			}

			if (tiling == vk::ImageTiling::eOptimal &&
				(fmt_properties.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		throw ENGINE_EXCEPTION("failed to find supported format!");
	}

	vk::Format PhysicalDevice::findSupportedDepthFormat() const
	{
		return findSupportedFormat(
			{
				vk::Format::eD32Sfloat,
				vk::Format::eD32SfloatS8Uint,
				vk::Format::eD24UnormS8Uint,
			},
			vk::ImageTiling::eOptimal,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment
		);
	}

	std::optional<QueueFamilyIndices> PhysicalDevice::findVulkanQueueFamilies(
		const Surface* with_surface
	) const
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
			if (with_surface->queryQueueSupport(*this, indice))
			{
				indices.present_family = indice;
				present_found		   = true;
			}

			indice++;
		}

		if (graphics_found && present_found) { return indices; }

		return std::nullopt;
	}

#pragma endregion
} // namespace Engine::Rendering::Vulkan
