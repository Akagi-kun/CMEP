#include "Wrappers/PhysicalDevice.hpp"

#include "Wrappers/Surface.hpp"

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

	VkFormat PhysicalDevice::FindSupportedFormat(
		const std::vector<VkFormat>& candidates,
		VkImageTiling tiling,
		VkFormatFeatureFlags features
	) const
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(native_handle, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}

			if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		throw std::runtime_error("failed to find supported VkFormat!");
	}

	VkFormat PhysicalDevice::FindSupportedDepthFormat() const
	{
		return FindSupportedFormat(
			{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
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
