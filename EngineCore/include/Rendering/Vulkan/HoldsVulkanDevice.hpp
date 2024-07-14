#pragma once

#include "framework.hpp"

namespace Engine::Rendering::Vulkan
{
	class HoldsVulkanDevice
	{
	protected:
		VDeviceManager* const device_manager;

	public:
		HoldsVulkanDevice() = delete;
		HoldsVulkanDevice(VDeviceManager* const with_device_manager) : device_manager(with_device_manager)
		{
		}
	};
} // namespace Engine::Rendering::Vulkan
