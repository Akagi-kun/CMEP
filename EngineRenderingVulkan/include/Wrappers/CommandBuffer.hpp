#pragma once

#include "HandleWrapper.hpp"
#include "ImportVulkan.hpp"
#include "InstanceOwned.hpp"
#include "Wrappers/framework.hpp"

#include <functional>

namespace Engine::Rendering::Vulkan
{
	class CommandBuffer final : public InstanceOwned, public HandleWrapper<vk::raii::CommandBuffer>
	{
	public:
		CommandBuffer(InstanceOwned::value_t with_instance, vk::raii::CommandPool& from_pool);
		~CommandBuffer();

		void BeginCmdBuffer(vk::CommandBufferUsageFlags usage_flags);
		void EndCmdBuffer();

		void ResetBuffer(vk::CommandBufferResetFlags flags = {});

		// TODO: Remove this in the future
		void RecordCmds(std::function<void(CommandBuffer*)> const& lambda);

		void QueueSubmit(const vk::raii::Queue& to_queue);

		void BufferBufferCopy(Buffer* from_buffer, Buffer* to_buffer, std::vector<vk::BufferCopy> regions);
		void BufferImageCopy(Buffer* from_buffer, Image* to_image);

		void BeginRenderPass(const vk::RenderPassBeginInfo& with_info);
		void EndRenderPass();

	private:
		vk::CommandPool owning_pool = nullptr;
	};
} // namespace Engine::Rendering::Vulkan
