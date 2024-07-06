#pragma once

#include "HoldsVulkanDevice.hpp"
#include "framework.hpp"
#include "vulkan/vulkan_core.h"

#include <functional>

namespace Engine::Rendering::Vulkan
{
	class VCommandBuffer final : public HoldsVulkanDevice
	{
	private:
		VCommandPool* owning_pool	  = nullptr;
		VkCommandBuffer native_handle = VK_NULL_HANDLE;

	public:
		VCommandBuffer(VulkanDeviceManager* with_device_manager, VCommandPool* from_pool);
		~VCommandBuffer();

		void BeginCmdBuffer(VkCommandBufferUsageFlags usage_flags);
		void EndCmdBuffer();

		void RecordCmds(std::function<void(VCommandBuffer*)> const& lambda);

		void GraphicsQueueSubmit();

		[[nodiscard]] VkCommandBuffer& GetNativeHandle()
		{
			return this->native_handle;
		}
	};
} // namespace Engine::Rendering::Vulkan
