#include "backend/DeviceScore.hpp"

#include "rendering/Surface.hpp"

#include <set>
#include <string>
#include <vector>

namespace Engine::Rendering::Vulkan
{
	const std::vector<const char*> DeviceScore::device_extensions = {
		vk::KHRSwapchainExtensionName,
		vk::EXTRobustness2ExtensionName,
	};

	DeviceScore::DeviceScore(const PhysicalDevice& with_device, const Surface& with_surface)
		: device_scored(with_device)
	{
		vk::PhysicalDeviceProperties device_properties = with_device.getProperties();

		vk::PhysicalDeviceFeatures device_features = with_device.getFeatures();

		// Suppress magic numbers since it doesnt make sense here
		// NOLINTBEGIN(readability-magic-numbers)
		//
		// Naive scoring to favor usually-more-performant devices
		switch (device_properties.deviceType)
		{
			case vk::PhysicalDeviceType::eDiscreteGpu:	 preference_score += 32; break;
			case vk::PhysicalDeviceType::eIntegratedGpu: preference_score += 16; break;
			case vk::PhysicalDeviceType::eVirtualGpu:	 preference_score += 8; break;
			case vk::PhysicalDeviceType::eCpu:			 preference_score += 4; break;
			default:									 preference_score += 2; break;
		}
		// NOLINTEND(readability-magic-numbers)

		auto indices = with_device.findVulkanQueueFamilies(with_surface);

		// Return (unsuitable) if some of the required functions aren't supported
		if (!indices.has_value())
		{
			unsupported_reason = "required queue types unsupported";
			return;
		}

		if (!checkDeviceExtensionSupport(with_device))
		{
			unsupported_reason = "required extensions unsupported";
			return;
		}

		if (device_features.samplerAnisotropy == 0u)
		{
			unsupported_reason = "feature samplerAnisotropy unsupported";
			return;
		}

		SwapChainSupportDetails swap_chain_support = with_surface.querySwapChainSupport(with_device);

		bool swap_chain_adequate = !swap_chain_support.formats.empty() &&
								   !swap_chain_support.present_modes.empty();

		if (!swap_chain_adequate)
		{
			unsupported_reason = "inadequate or no swap chain support";
			return;
		}

		// Device supported, check suitability
		supported = true;
	}

	bool DeviceScore::checkDeviceExtensionSupport(const vk::raii::PhysicalDevice& device)
	{
		std::vector<vk::ExtensionProperties> available_extensions =
			device.enumerateDeviceExtensionProperties();

		std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());

		for (const auto& extension : available_extensions)
		{
			required_extensions.erase(extension.extensionName);
		}

		return required_extensions.empty();
	}
} // namespace Engine::Rendering::Vulkan
