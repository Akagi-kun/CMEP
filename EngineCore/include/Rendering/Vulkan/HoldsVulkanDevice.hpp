#pragma once

#include "framework.hpp"

namespace Engine::Rendering::Vulkan
{
	class HoldsVulkanDevice
	{
	protected:
		VulkanDeviceManager* const device_manager;

	public:
		HoldsVulkanDevice() = delete;
		HoldsVulkanDevice(VulkanDeviceManager* const with_device_manager) : device_manager(with_device_manager)
		{
		}
	};
} // namespace Engine::Rendering::Vulkan
