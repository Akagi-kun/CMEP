#pragma once

#include "HoldsVulkanDevice.hpp"
#include "framework.hpp"
#include "vulkan/vulkan_core.h"

namespace Engine::Rendering::Vulkan
{
	struct RenderPass final : HoldsVulkanDevice
	{
		VkRenderPass native_handle;

		RenderPass(DeviceManager* with_device_manager, VkFormat with_format);
		~RenderPass();
	};
} // namespace Engine::Rendering::Vulkan
