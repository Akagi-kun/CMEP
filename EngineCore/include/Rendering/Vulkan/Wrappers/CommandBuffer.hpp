#pragma once

#include "Rendering/Vulkan/Wrappers/framework.hpp"

#include "HandleWrapper.hpp"
#include "InstanceOwned.hpp"
#include "vulkan/vulkan_core.h"

#include <functional>

namespace Engine::Rendering::Vulkan
{
	class CommandBuffer final : public InstanceOwned, public HandleWrapper<VkCommandBuffer>
	{
	public:
		CommandBuffer(InstanceOwned::value_t with_instance, VkCommandPool from_pool);
		~CommandBuffer();

		void BeginCmdBuffer(VkCommandBufferUsageFlags usage_flags);
		void EndCmdBuffer();

		void ResetBuffer(VkCommandBufferResetFlags flags = 0);

		// TODO: Remove this in the future
		void RecordCmds(std::function<void(CommandBuffer*)> const& lambda);

		void QueueSubmit(VkQueue to_queue);

		void BufferBufferCopy(Buffer* from_buffer, Buffer* to_buffer, std::vector<VkBufferCopy> regions);
		void BufferImageCopy(Buffer* from_buffer, Image* to_image);

		void BeginRenderPass(const VkRenderPassBeginInfo* with_info);
		void EndRenderPass();

	private:
		VkCommandPool owning_pool = nullptr;
	};
} // namespace Engine::Rendering::Vulkan