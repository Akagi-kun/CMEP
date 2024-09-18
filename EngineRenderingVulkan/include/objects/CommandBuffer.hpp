#pragma once

#include "fwd.hpp"

#include "common/HandleWrapper.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <vector>

namespace Engine::Rendering::Vulkan
{
	class CommandBuffer final : public vk::raii::CommandBuffer
	{
	public:
		CommandBuffer(LogicalDevice* with_device, vk::raii::CommandPool& from_pool);
		~CommandBuffer() = default;

		void copyBufferBuffer(
			const vk::raii::Queue&			   in_queue,
			Buffer*							   from_buffer,
			Buffer*							   to_buffer,
			const std::vector<vk::BufferCopy>& regions
		);
		void
		copyBufferImage(const vk::raii::Queue& in_queue, Buffer* from_buffer, Image* to_image);

		static vk::CommandBufferBeginInfo
		getBeginInfo(vk::CommandBufferUsageFlags usage_flags);

	private:
		LogicalDevice* device;
	};
} // namespace Engine::Rendering::Vulkan
