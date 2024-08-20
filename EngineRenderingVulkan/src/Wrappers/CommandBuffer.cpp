#include "Wrappers/CommandBuffer.hpp"

#include "ImportVulkan.hpp"
#include "VulkanUtilities.hpp"
#include "Wrappers/Buffer.hpp"
#include "Wrappers/Image.hpp"
#include "Wrappers/Instance.hpp"
#include "Wrappers/InstanceOwned.hpp"
#include "Wrappers/LogicalDevice.hpp"

namespace Engine::Rendering::Vulkan
{
	CommandBuffer::CommandBuffer(InstanceOwned::value_t with_instance, vk::CommandPool from_pool)
		: InstanceOwned(with_instance), owning_pool(from_pool)
	{
		vk::CommandBufferAllocateInfo alloc_info(from_pool, vk::CommandBufferLevel::ePrimary, 1, {});

		LogicalDevice* logical_device = instance->GetLogicalDevice();

		native_handle = logical_device->GetHandle().allocateCommandBuffers(alloc_info)[0];
	}

	CommandBuffer::~CommandBuffer()
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		logical_device->GetHandle().freeCommandBuffers(owning_pool, native_handle);
	}

	void CommandBuffer::BeginCmdBuffer(vk::CommandBufferUsageFlags usage_flags)
	{
		ResetBuffer();

		native_handle.begin({usage_flags, {}, {}});
	}

	void CommandBuffer::EndCmdBuffer()
	{
		native_handle.end();
	}

	void CommandBuffer::ResetBuffer(vk::CommandBufferResetFlags flags)
	{
		native_handle.reset(flags);
	}

	void CommandBuffer::RecordCmds(std::function<void(CommandBuffer*)> const& lambda)
	{
		LogicalDevice* device = instance->GetLogicalDevice();

		// TODO: Check if we should really pass non-zero here
		BeginCmdBuffer(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		// TODO: Don't pass this here
		lambda(this);

		EndCmdBuffer();
		QueueSubmit(device->GetGraphicsQueue());

		device->GetGraphicsQueue().WaitQueueIdle();
	}

	void CommandBuffer::QueueSubmit(vk::Queue to_queue)
	{
		vk::SubmitInfo submit_info{};
		submit_info.setCommandBuffers(native_handle);

		to_queue.submit(submit_info);
	}

	void CommandBuffer::BufferBufferCopy(Buffer* from_buffer, Buffer* to_buffer, std::vector<VkBufferCopy> regions)
	{
		RecordCmds([&](CommandBuffer* handle) {
			vkCmdCopyBuffer(
				handle->native_handle,
				from_buffer->GetHandle(),
				to_buffer->GetHandle(),
				static_cast<uint32_t>(regions.size()),
				regions.data()
			);
		});
	}

	void CommandBuffer::BufferImageCopy(Buffer* from_buffer, Image* to_image)
	{
		ImageSize image_size = to_image->GetSize();

		// Sane defaults
		VkBufferImageCopy region{};
		region.bufferOffset		 = 0;
		region.bufferRowLength	 = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask	   = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel	   = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount	   = 1;

		region.imageOffset = {0, 0, 0};
		region.imageExtent = Utils::ConvertToExtent<vk::Extent3D>(image_size, 1);

		RecordCmds([&](CommandBuffer* with_buf) {
			vkCmdCopyBufferToImage(
				with_buf->native_handle,
				from_buffer->GetHandle(),
				static_cast<VkImage>(to_image->GetHandle()),
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&region
			);
		});
	}

	void CommandBuffer::BeginRenderPass(const vk::RenderPassBeginInfo* with_info)
	{
		native_handle.beginRenderPass(with_info, vk::SubpassContents::eInline);
	}

	void CommandBuffer::EndRenderPass()
	{
		native_handle.endRenderPass();
	}

} // namespace Engine::Rendering::Vulkan
