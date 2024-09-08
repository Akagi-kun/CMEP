#include "rendering/Surface.hpp"

#include <cstdint>

namespace Engine::Rendering::Vulkan
{
#pragma region Public

	SwapChainSupportDetails Surface::querySwapChainSupport(const vk::raii::PhysicalDevice& device
	) const
	{
		SwapChainSupportDetails details;
		details.capabilities  = device.getSurfaceCapabilitiesKHR(native_handle);
		details.formats		  = device.getSurfaceFormatsKHR(native_handle);
		details.present_modes = device.getSurfacePresentModesKHR(native_handle);

		return details;
	}

	bool Surface::queryQueueSupport(
		const vk::raii::PhysicalDevice& physical_device,
		uint32_t						queue_family
	) const
	{
		vk::Bool32 support = physical_device.getSurfaceSupportKHR(queue_family, native_handle);

		return support != 0u;
	}

#pragma endregion
} // namespace Engine::Rendering::Vulkan
