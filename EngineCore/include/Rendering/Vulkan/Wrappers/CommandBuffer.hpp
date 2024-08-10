#pragma once

#include "Rendering/Vulkan/Wrappers/HoldsVulkanDevice.hpp"
#include "Rendering/Vulkan/Wrappers/framework.hpp"

#include "vulkan/vulkan_core.h"

#include <functional>

namespace Engine::Rendering::Vulkan
{
	class CommandBuffer final : public HoldsVulkanDevice
	{
	private:
		VkCommandPool owning_pool	  = nullptr;
		VkCommandBuffer native_handle = VK_NULL_HANDLE;

	public:
		CommandBuffer(DeviceManager* with_device_manager, VkCommandPool from_pool);
		~CommandBuffer();

		void BeginCmdBuffer(VkCommandBufferUsageFlags usage_flags);
		void EndCmdBuffer();

		void ResetBuffer();

		// TODO: Remove this in the future
		void RecordCmds(std::function<void(CommandBuffer*)> const& lambda);

		void QueueSubmit(VkQueue to_queue);

		void BufferBufferCopy(Buffer* from_buffer, Buffer* to_buffer, std::vector<VkBufferCopy> regions);
		void BufferImageCopy(Buffer* from_buffer, Image* to_image);

		[[nodiscard]] VkCommandBuffer& GetNativeHandle()
		{
			return this->native_handle;
		}

		void BeginRenderPass(const VkRenderPassBeginInfo* with_info);
		void EndRenderPass();
	};
} // namespace Engine::Rendering::Vulkan
