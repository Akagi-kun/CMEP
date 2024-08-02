#pragma once

#include "HoldsVulkanDevice.hpp"
#include "framework.hpp"
#include "vulkan/vulkan_core.h"

#include <cassert>

namespace Engine::Rendering::Vulkan
{
	class VCommandPool : public HoldsVulkanDevice
	{
	protected:
		VkCommandPool native_handle = VK_NULL_HANDLE;

	public:
		VCommandPool(VDeviceManager* with_device_manager);

		~VCommandPool();

		[[nodiscard]] VCommandBuffer* AllocateCommandBuffer();

		[[nodiscard]] VkCommandPool& GetNativeHandle()
		{
			assert(this->native_handle != VK_NULL_HANDLE && "This command pool has no valid native handle!");

			return this->native_handle;
		}
	};
} // namespace Engine::Rendering::Vulkan
