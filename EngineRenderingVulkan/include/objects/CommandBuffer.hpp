#pragma once

#include "fwd.hpp"

#include "ImportVulkan.hpp"
#include "common/HandleWrapper.hpp"

#include <functional>

namespace Engine::Rendering::Vulkan
{
	class CommandBuffer final : public HandleWrapper<vk::raii::CommandBuffer>
	{
	public:
		CommandBuffer(LogicalDevice* with_device, vk::raii::CommandPool& from_pool);
		~CommandBuffer() = default;

		// void BeginCmdBuffer(vk::CommandBufferUsageFlags usage_flags);

		// TODO: Remove this in the future
		void RecordCmds(std::function<void(vk::raii::CommandBuffer*)> const& lambda);

		// void QueueSubmit(const vk::raii::Queue& to_queue);

		void BufferBufferCopy(Buffer* from_buffer, Buffer* to_buffer, std::vector<vk::BufferCopy> regions);
		void BufferImageCopy(Buffer* from_buffer, Image* to_image);

		static vk::CommandBufferBeginInfo GetBeginInfo(vk::CommandBufferUsageFlags usage_flags);

	private:
		LogicalDevice* device;
	};
} // namespace Engine::Rendering::Vulkan
