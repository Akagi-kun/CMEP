#pragma once

#include "VulkanCommandPool.hpp"

#include <functional>

namespace Engine::Rendering
{
	class VulkanCommandBuffer : public HoldsVulkanDevice
	{
	private:
		VulkanCommandPool* owning_pool = nullptr;

	protected:
		VkCommandBuffer native_handle = VK_NULL_HANDLE;

	public:
		VulkanCommandBuffer(VulkanDeviceManager* with_device_manager, VulkanCommandPool* from_pool);
		~VulkanCommandBuffer();

		void BeginCmdBuffer(VkCommandBufferUsageFlags usage_flags);
		void EndCmdBuffer();

		void RecordCmds(std::function<void(VulkanCommandBuffer*)> const& lambda);

		void GraphicsQueueSubmit();

		[[nodiscard]] VkCommandBuffer& GetNativeHandle()
		{
			return this->native_handle;
		}
	};
} // namespace Engine::Rendering
