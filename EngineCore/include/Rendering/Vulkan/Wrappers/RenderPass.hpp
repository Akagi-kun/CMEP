#pragma once

#include "HoldsVulkanDevice.hpp"
#include "framework.hpp"
#include "vulkan/vulkan_core.h"

namespace Engine::Rendering::Vulkan
{
	class RenderPass final : HoldsVulkanDevice
	{
	private:
		VkRenderPass native_handle;

		friend class VSwapchain;
		friend class VPipeline;

	public:
		RenderPass(VDeviceManager* with_device_manager, VkFormat with_format);
		~RenderPass();

		/* [[nodiscard]] VkRenderPass GetNativeHandle()
		{
			return this->native_handle;
		} */
	};
} // namespace Engine::Rendering::Vulkan
