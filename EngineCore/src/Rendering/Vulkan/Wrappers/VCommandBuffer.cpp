#include "Rendering/Vulkan/Wrappers/VCommandBuffer.hpp"

#include "Rendering/Vulkan/VDeviceManager.hpp"
#include "Rendering/Vulkan/Wrappers/VBuffer.hpp"
#include "Rendering/Vulkan/Wrappers/VImage.hpp"

#include "vulkan/vulkan_core.h"

#include <stdexcept>

namespace Engine::Rendering::Vulkan
{
	VCommandBuffer::VCommandBuffer(VDeviceManager* const with_device_manager, VkCommandPool from_pool)
		: HoldsVulkanDevice(with_device_manager), owning_pool(from_pool)
	{
		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType			  = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool		  = from_pool;
		alloc_info.level			  = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = 1;

		VkDevice logical_device = this->device_manager->GetLogicalDevice();

		if (vkAllocateCommandBuffers(logical_device, &alloc_info, &this->native_handle) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	VCommandBuffer::~VCommandBuffer()
	{
		vkFreeCommandBuffers(this->device_manager->GetLogicalDevice(), this->owning_pool, 1, &this->native_handle);
	}

	void VCommandBuffer::BeginCmdBuffer(VkCommandBufferUsageFlags usage_flags)
	{
		this->ResetBuffer();

		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = usage_flags; // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(this->native_handle, &begin_info);
	}

	void VCommandBuffer::EndCmdBuffer()
	{
		vkEndCommandBuffer(this->native_handle);
	}

	void VCommandBuffer::ResetBuffer()
	{
		vkResetCommandBuffer(this->native_handle, 0);
	}

	void VCommandBuffer::RecordCmds(std::function<void(VCommandBuffer*)> const& lambda)
	{
		// TODO: Check if we should really pass non-zero here
		this->BeginCmdBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		// TODO: Don't pass this here
		lambda(this);

		this->EndCmdBuffer();
		this->QueueSubmit(this->device_manager->GetGraphicsQueue());

		vkQueueWaitIdle(this->device_manager->GetGraphicsQueue());
	}

	void VCommandBuffer::QueueSubmit(VkQueue to_queue)
	{
		VkSubmitInfo submit_info{};
		submit_info.sType			   = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers	   = &this->native_handle;

		vkQueueSubmit(to_queue, 1, &submit_info, VK_NULL_HANDLE);
	}

	void VCommandBuffer::BufferBufferCopy(VBuffer* from_buffer, VBuffer* to_buffer, std::vector<VkBufferCopy> regions)
	{
		this->RecordCmds([&](VCommandBuffer* handle) {
			vkCmdCopyBuffer(
				handle->native_handle,
				from_buffer->GetNativeHandle(),
				to_buffer->GetNativeHandle(),
				static_cast<uint32_t>(regions.size()),
				regions.data()
			);
		});
	}

	void VCommandBuffer::BufferImageCopy(VBuffer* from_buffer, VImage* to_image)
	{
		VImageSize image_size = to_image->GetSize();

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

		this->RecordCmds([&](VCommandBuffer* with_buf) {
			vkCmdCopyBufferToImage(
				with_buf->GetNativeHandle(),
				from_buffer->GetNativeHandle(),
				to_image->GetNativeHandle(),
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&region
			);
		});
	}

	void VCommandBuffer::BeginRenderPass(const VkRenderPassBeginInfo* with_info)
	{
		vkCmdBeginRenderPass(this->native_handle, with_info, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VCommandBuffer::EndRenderPass()
	{
		vkCmdEndRenderPass(this->native_handle);
	}

} // namespace Engine::Rendering::Vulkan
