#pragma once

#include "fwd.hpp"

#include "ImportVulkan.hpp"
#include "common/HandleWrapper.hpp"
#include "common/InstanceOwned.hpp"

#include <functional>

namespace Engine::Rendering::Vulkan
{
	class CommandBuffer final : public InstanceOwned, public HandleWrapper<vk::raii::CommandBuffer>
	{
	public:
		CommandBuffer(InstanceOwned::value_t with_instance, vk::raii::CommandPool& from_pool);
		~CommandBuffer() = default;

		void BeginCmdBuffer(vk::CommandBufferUsageFlags usage_flags);

		// TODO: Remove this in the future
		void RecordCmds(std::function<void(CommandBuffer*)> const& lambda);

		void QueueSubmit(const vk::raii::Queue& to_queue);

		void BufferBufferCopy(Buffer* from_buffer, Buffer* to_buffer, std::vector<vk::BufferCopy> regions);
		void BufferImageCopy(Buffer* from_buffer, Image* to_image);

	private:
		vk::CommandPool owning_pool = nullptr;
	};
} // namespace Engine::Rendering::Vulkan
