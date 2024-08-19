#include "Wrappers/Surface.hpp"

#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_structs.hpp"

namespace Engine::Rendering::Vulkan
{
#pragma region Public

	SwapChainSupportDetails Surface::QueryVulkanSwapChainSupport(vk::PhysicalDevice device) const
	{
		assert(device && "Tried to query swapchain support with invalid vk::PhysicalDevice");

		SwapChainSupportDetails details;
		details.capabilities  = device.getSurfaceCapabilitiesKHR(native_handle);
		details.formats		  = device.getSurfaceFormatsKHR(native_handle);
		details.present_modes = device.getSurfacePresentModesKHR(native_handle);

		return details;
	}

	bool Surface::QueryQueueSupport(vk::PhysicalDevice physical_device, uint32_t queue_family) const
	{
		vk::Bool32 support = physical_device.getSurfaceSupportKHR(queue_family, native_handle);

		return support != 0u;
	}

	QueueFamilyIndices Surface::FindVulkanQueueFamilies(vk::PhysicalDevice device) const
	{
		QueueFamilyIndices indices;

		std::vector<vk::QueueFamilyProperties> queue_families = device.getQueueFamilyProperties();

		uint32_t indice = 0;
		for (const auto& queue_family : queue_families)
		{
			if ((queue_family.queueFlags & vk::QueueFlagBits::eGraphics /* VK_QUEUE_GRAPHICS_BIT */))
			{
				indices.graphics_family = indice;
			}

			if (this->QueryQueueSupport(device, indice))
			{
				indices.present_family = indice;
			}

			indice++;
		}

		return indices;
	}
#pragma endregion
} // namespace Engine::Rendering::Vulkan
