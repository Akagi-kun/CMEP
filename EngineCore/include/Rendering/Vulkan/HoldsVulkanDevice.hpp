#pragma once

#include "Rendering/Vulkan/VulkanDeviceManager.hpp"

namespace Engine::Rendering
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
} // namespace Engine::Rendering
