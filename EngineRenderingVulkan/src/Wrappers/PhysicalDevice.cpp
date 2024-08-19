#include "Wrappers/PhysicalDevice.hpp"

#include "Wrappers/Surface.hpp"
#include "vulkan/vulkan_enums.hpp"

#include <stdexcept>

namespace Engine::Rendering::Vulkan
{
#pragma region Public

	std::string PhysicalDevice::GetDeviceName() const
	{
		VkPhysicalDeviceProperties device_properties;
		vkGetPhysicalDeviceProperties(native_handle, &device_properties);

		return device_properties.deviceName;
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
			/* VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(native_handle, format, &props); */

			if (tiling == vk::ImageTiling::eLinear /* VK_IMAGE_TILING_LINEAR */ &&
				(fmt_properties.linearTilingFeatures & features) == features)
			{
				return format;
			}

			if (tiling == vk::ImageTiling::eOptimal /* VK_IMAGE_TILING_OPTIMAL */ &&
				(fmt_properties.optimalTilingFeatures & features) == features)
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

		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(native_handle, &queue_family_count, nullptr);

		std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(native_handle, &queue_family_count, queue_families.data());

		uint32_t indice = 0;
		for (const auto& queue_family : queue_families)
		{
			if ((queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
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
