#include "Rendering/Vulkan/Wrappers/CommandBuffer.hpp"

#include "Rendering/Vulkan/Wrappers/Buffer.hpp"
#include "Rendering/Vulkan/Wrappers/Image.hpp"
#include "Rendering/Vulkan/Wrappers/Instance.hpp"
#include "Rendering/Vulkan/Wrappers/InstanceOwned.hpp"
#include "Rendering/Vulkan/Wrappers/LogicalDevice.hpp"

#include "vulkan/vulkan_core.h"

#include <stdexcept>

namespace Engine::Rendering::Vulkan
{
	CommandBuffer::CommandBuffer(InstanceOwned::value_t with_instance, VkCommandPool from_pool)
		: InstanceOwned(with_instance), owning_pool(from_pool)
	{
		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType			  = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool		  = from_pool;
		alloc_info.level			  = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = 1;

		LogicalDevice* logical_device = instance->GetLogicalDevice();

		if (vkAllocateCommandBuffers(*logical_device, &alloc_info, &native_handle) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	CommandBuffer::~CommandBuffer()
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		vkFreeCommandBuffers(*logical_device, this->owning_pool, 1, &native_handle);
	}

	void CommandBuffer::BeginCmdBuffer(VkCommandBufferUsageFlags usage_flags)
	{
		ResetBuffer();

		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = usage_flags; // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(native_handle, &begin_info);
	}

	void CommandBuffer::EndCmdBuffer()
	{
		vkEndCommandBuffer(native_handle);
	}

	void CommandBuffer::ResetBuffer(VkCommandBufferResetFlags flags)
	{
		vkResetCommandBuffer(native_handle, flags);
	}

	void CommandBuffer::RecordCmds(std::function<void(CommandBuffer*)> const& lambda)
	{
		LogicalDevice* device = instance->GetLogicalDevice();

		// TODO: Check if we should really pass non-zero here
		this->BeginCmdBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		// TODO: Don't pass this here
		lambda(this);

		this->EndCmdBuffer();
		this->QueueSubmit(device->GetGraphicsQueue());

		device->GetGraphicsQueue().WaitQueueIdle();
	}

	void CommandBuffer::QueueSubmit(VkQueue to_queue)
	{
		VkSubmitInfo submit_info{};
		submit_info.sType			   = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers	   = &native_handle;

		vkQueueSubmit(to_queue, 1, &submit_info, VK_NULL_HANDLE);
	}

	void CommandBuffer::BufferBufferCopy(Buffer* from_buffer, Buffer* to_buffer, std::vector<VkBufferCopy> regions)
	{
		this->RecordCmds([&](CommandBuffer* handle) {
			vkCmdCopyBuffer(
				handle->native_handle,
				*from_buffer,
				*to_buffer,
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
		region.imageExtent = {image_size.x, image_size.y, 1};

		this->RecordCmds([&](CommandBuffer* with_buf) {
			vkCmdCopyBufferToImage(
				with_buf->native_handle,
				*from_buffer,
				*to_image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&region
			);
		});
	}

	void CommandBuffer::BeginRenderPass(const VkRenderPassBeginInfo* with_info)
	{
		vkCmdBeginRenderPass(native_handle, with_info, VK_SUBPASS_CONTENTS_INLINE);
	}

	void CommandBuffer::EndRenderPass()
	{
		vkCmdEndRenderPass(native_handle);
	}

} // namespace Engine::Rendering::Vulkan
