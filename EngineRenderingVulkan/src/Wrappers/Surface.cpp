#include "Wrappers/Surface.hpp"

namespace Engine::Rendering::Vulkan
{
#pragma region Public

	SwapChainSupportDetails Surface::QueryVulkanSwapChainSupport(VkPhysicalDevice device) const
	{
		assert(device != nullptr && "Tried to query swapchain support with VkPhysicalDevice == nullptr");

		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, native_handle, &details.capabilities);

		uint32_t format_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, native_handle, &format_count, nullptr);

		if (format_count != 0)
		{
			details.formats.resize(format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, native_handle, &format_count, details.formats.data());
		}

		uint32_t present_mode_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, native_handle, &present_mode_count, nullptr);

		if (present_mode_count != 0)
		{
			details.present_modes.resize(present_mode_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(
				device,
				native_handle,
				&present_mode_count,
				details.present_modes.data()
			);
		}

		return details;
	}

	bool Surface::QueryQueueSupport(VkPhysicalDevice physical_device, uint32_t queue_family) const
	{
		VkBool32 support;
		vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, queue_family, native_handle, &support);

		return support != 0u;
	}

	QueueFamilyIndices Surface::FindVulkanQueueFamilies(VkPhysicalDevice device) const
	{
		QueueFamilyIndices indices;

		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

		std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

		uint32_t indice = 0;
		for (const auto& queue_family : queue_families)
		{
			if ((queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
			{
				indices.graphics_family = indice;
			}

			if (this->QueryQueueSupport(device, indice)) // (present_support != 0u)
			{
				indices.present_family = indice;
			}

			indice++;
		}

		return indices;
	}
#pragma endregion
} // namespace Engine::Rendering::Vulkan
