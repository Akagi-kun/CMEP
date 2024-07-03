#include "Rendering/Vulkan/VulkanCommandBuffer.hpp"

#include "vulkan/vulkan_core.h"

namespace Engine::Rendering
{
	VulkanCommandBuffer::VulkanCommandBuffer(
		VulkanDeviceManager* const with_device_manager,
		VulkanCommandPool* from_pool
	)
		: HoldsVulkanDevice(with_device_manager), owning_pool(from_pool)
	{
		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType			  = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool		  = from_pool->GetNativeHandle();
		alloc_info.level			  = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = 1;

		VkDevice logical_device = this->device_manager->GetLogicalDevice();

		if (vkAllocateCommandBuffers(logical_device, &alloc_info, &this->native_handle) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		vkFreeCommandBuffers(
			this->device_manager->GetLogicalDevice(),
			this->owning_pool->GetNativeHandle(),
			1,
			&this->native_handle
		);
	}

	void VulkanCommandBuffer::BeginCmdBuffer(VkCommandBufferUsageFlags usage_flags)
	{
		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = usage_flags; // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(this->native_handle, &begin_info);
	}

	void VulkanCommandBuffer::EndCmdBuffer()
	{
		vkEndCommandBuffer(this->native_handle);
	}

	void VulkanCommandBuffer::RecordCmds(std::function<void(VulkanCommandBuffer*)> const& lambda)
	{
		this->BeginCmdBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		lambda(this);

		this->EndCmdBuffer();
		this->GraphicsQueueSubmit();

		vkQueueWaitIdle(this->device_manager->GetGraphicsQueue());

		vkResetCommandBuffer(this->native_handle, 0);
	}

	void VulkanCommandBuffer::GraphicsQueueSubmit()
	{
		VkSubmitInfo submit_info{};
		submit_info.sType			   = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers	   = &this->native_handle;

		vkQueueSubmit(this->device_manager->GetGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE);
	}

} // namespace Engine::Rendering
