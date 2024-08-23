#include "objects/CommandBuffer.hpp"

#include "ImportVulkan.hpp"
#include "backend/LogicalDevice.hpp"
#include "common/Utilities.hpp"
#include "objects/Buffer.hpp"
#include "objects/Image.hpp"

namespace Engine::Rendering::Vulkan
{
	CommandBuffer::CommandBuffer(LogicalDevice* with_device, vk::raii::CommandPool& from_pool) : device(with_device)
	{
		vk::CommandBufferAllocateInfo alloc_info(*from_pool, vk::CommandBufferLevel::ePrimary, 1, {});

		native_handle = std::move(device->allocateCommandBuffers(alloc_info)[0]);
	}

	vk::CommandBufferBeginInfo CommandBuffer::GetBeginInfo(vk::CommandBufferUsageFlags usage_flags)
	{
		return {usage_flags, {}, {}};
	}

	void CommandBuffer::RecordCmds(std::function<void(vk::raii::CommandBuffer*)> const& lambda)
	{
		// TODO: Check if we should really pass non-zero here
		native_handle.begin(GetBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

		lambda(&native_handle);

		native_handle.end();
		device->GetGraphicsQueue().submit(vk::SubmitInfo().setCommandBuffers(*native_handle));

		device->GetGraphicsQueue().waitIdle();
	}

	void CommandBuffer::BufferBufferCopy(Buffer* from_buffer, Buffer* to_buffer, std::vector<vk::BufferCopy> regions)
	{
		RecordCmds([&](vk::raii::CommandBuffer* handle) {
			handle->copyBuffer(*from_buffer->GetHandle(), *to_buffer->GetHandle(), regions);
		});
	}

	void CommandBuffer::BufferImageCopy(Buffer* from_buffer, Image* to_image)
	{
		ImageSize image_size = to_image->GetSize();

		// Sane defaults
		vk::BufferImageCopy region(
			{},
			{},
			{},
			{vk::ImageAspectFlagBits::eColor, 0, 0, 1},
			{0, 0, 0},
			Utility::ConvertToExtent<vk::Extent3D>(image_size, 1)
		);

		RecordCmds([&](vk::raii::CommandBuffer* with_buf) {
			with_buf->copyBufferToImage(
				*from_buffer->GetHandle(),
				*to_image->GetHandle(),
				vk::ImageLayout::eTransferDstOptimal,
				region
			);
		});
	}

} // namespace Engine::Rendering::Vulkan
