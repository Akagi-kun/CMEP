#include "Rendering/Vulkan/Wrappers/DeviceScore.hpp"

#include "Rendering/Vulkan/Wrappers/Surface.hpp"

#include <set>

namespace Engine::Rendering::Vulkan
{
	const std::vector<const char*> DeviceScore::device_extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
		VK_EXT_ROBUSTNESS_2_EXTENSION_NAME,
	};

	DeviceScore::DeviceScore(PhysicalDevice with_device, const Surface* const with_surface) : device_scored(with_device)
	{
		// Physical device properties
		VkPhysicalDeviceProperties device_properties;
		vkGetPhysicalDeviceProperties(with_device, &device_properties);

		// Physical device optional features
		VkPhysicalDeviceFeatures device_features;
		vkGetPhysicalDeviceFeatures(with_device, &device_features);

		// Suppress magic numbers since it doesnt make sense here
		// NOLINTBEGIN(readability-magic-numbers)
		//
		// Naive scoring to favor usually-more-performant devices
		switch (device_properties.deviceType)
		{
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				preference_score = 12;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				preference_score = 10;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
				preference_score = 8;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_CPU:
				preference_score = 4;
				break;
			default:
				preference_score = 2;
				break;
		}
		// NOLINTEND(readability-magic-numbers)

		QueueFamilyIndices indices = with_surface->FindVulkanQueueFamilies(with_device);

		// Return 0 (unsuitable) if some of the required functions aren't supported
		if (!indices.IsComplete())
		{
			unsupported_reason = "required queue types unsupported";
			return;
		}

		if (!CheckDeviceExtensionSupport(with_device))
		{
			unsupported_reason = "required extensions unsupported";
			return;
		}

		if (device_features.samplerAnisotropy == 0u)
		{
			unsupported_reason = "feature samplerAnisotropy unsupported";
			return;
		}

		bool swap_chain_adequate				   = false;
		SwapChainSupportDetails swap_chain_support = with_surface->QueryVulkanSwapChainSupport(with_device);
		swap_chain_adequate = !swap_chain_support.formats.empty() && !swap_chain_support.present_modes.empty();

		if (!swap_chain_adequate)
		{
			unsupported_reason = "inadequate or no swap chain support";
			return;
		}

		// Device supported, check suitability
		supported = true;
	}

	bool DeviceScore::CheckDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extension_count;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

		std::vector<VkExtensionProperties> available_extensions(extension_count);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

		std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());

		for (const auto& extension : available_extensions)
		{
			required_extensions.erase(extension.extensionName);
		}

		return required_extensions.empty();
	}
} // namespace Engine::Rendering::Vulkan
