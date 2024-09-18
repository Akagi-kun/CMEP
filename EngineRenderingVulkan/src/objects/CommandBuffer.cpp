#include "objects/CommandBuffer.hpp"

#include "Rendering/Transform.hpp"

#include "backend/LogicalDevice.hpp"
#include "common/Utility.hpp"
#include "objects/Buffer.hpp"
#include "objects/Image.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <utility>
#include <vector>

namespace Engine::Rendering::Vulkan
{
	CommandBuffer::CommandBuffer(LogicalDevice* with_device, vk::raii::CommandPool& from_pool)
		: vk::raii::CommandBuffer(std::move(with_device->allocateCommandBuffers(
			  {.commandPool		   = *from_pool,
			   .level			   = vk::CommandBufferLevel::ePrimary,
			   .commandBufferCount = 1}
		  )[0])),
		  device(with_device)
	{}

	vk::CommandBufferBeginInfo CommandBuffer::getBeginInfo(
		vk::CommandBufferUsageFlags usage_flags
	)
	{
		return {.flags = usage_flags};
	}

	void CommandBuffer::copyBufferBuffer(
		const vk::raii::Queue&			   in_queue,
		Buffer*							   from_buffer,
		Buffer*							   to_buffer,
		const std::vector<vk::BufferCopy>& regions
	)
	{
		begin(getBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

		copyBuffer(*from_buffer->getHandle(), *to_buffer->getHandle(), regions);

		end();
		in_queue.submit(vk::SubmitInfo{.commandBufferCount = 1, .pCommandBuffers = &**this}
		);
		in_queue.waitIdle();
	}

	void CommandBuffer::copyBufferImage(
		const vk::raii::Queue& in_queue,
		Buffer*				   from_buffer,
		Image*				   to_image
	)
	{
		ImageSize image_size = to_image->getSize();

		// Sane defaults
		vk::BufferImageCopy region{
			.bufferOffset	   = {},
			.bufferRowLength   = {},
			.bufferImageHeight = {},
			.imageSubresource  = {vk::ImageAspectFlagBits::eColor, 0, 0, 1},
			.imageOffset	   = {0, 0, 0},
			.imageExtent	   = Utility::convertToExtent<vk::Extent3D>(image_size, 1)
		};

		begin(getBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
		copyBufferToImage(
			*from_buffer->getHandle(),
			*to_image->getHandle(),
			vk::ImageLayout::eTransferDstOptimal,
			region
		);
		end();

		in_queue.submit(vk::SubmitInfo{.commandBufferCount = 1, .pCommandBuffers = &**this}
		);
		in_queue.waitIdle();
	}

} // namespace Engine::Rendering::Vulkan
