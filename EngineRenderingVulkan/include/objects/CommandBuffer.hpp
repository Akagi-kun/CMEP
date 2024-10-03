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
			Buffer*							   from_buffer,
			Buffer*							   to_buffer,
			const std::vector<vk::BufferCopy>& regions
		);

		void copyBufferImage(Buffer* from_buffer, Image* to_image);

		static vk::CommandBufferBeginInfo
		getBeginInfo(vk::CommandBufferUsageFlags usage_flags);

		/**
		 * @brief Submit this command buffer to a queue.
		 *
		 * @param to_queue The queue to submit to
		 */
		void queueSubmit(vk::raii::Queue& to_queue);

		/**
		 * @brief Shortcut to calling begin with eOneTimeSubmit
		 */
		void beginOneTime()
		{
			begin(getBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
		}

	private:
		LogicalDevice* device;
	};
} // namespace Engine::Rendering::Vulkan
