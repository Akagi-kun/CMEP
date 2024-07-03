#pragma once

#include "HoldsVulkanDevice.hpp"
#include "vulkan/vulkan_core.h"

namespace Engine::Rendering
{
	class VulkanCommandPool : public HoldsVulkanDevice
	{
	protected:
		VkCommandPool native_handle = VK_NULL_HANDLE;

	public:
		VulkanCommandPool(VulkanDeviceManager* with_device_manager);

		~VulkanCommandPool();

		[[nodiscard]] VkCommandPool GetNativeHandle()
		{
			assert(this->native_handle != VK_NULL_HANDLE && "This command pool has no valid native handle!");

			return this->native_handle;
		}
	};
} // namespace Engine::Rendering
