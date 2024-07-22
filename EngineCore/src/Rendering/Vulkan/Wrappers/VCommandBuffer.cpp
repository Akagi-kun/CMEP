#include "Rendering/Vulkan/Wrappers/VCommandBuffer.hpp"

#include "Rendering/Vulkan/VDeviceManager.hpp"
#include "Rendering/Vulkan/Wrappers/VCommandPool.hpp"

#include "vulkan/vulkan_core.h"

#include <stdexcept>

namespace Engine::Rendering::Vulkan
{
	VCommandBuffer::VCommandBuffer(VDeviceManager* const with_device_manager, VCommandPool* from_pool)
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

	VCommandBuffer::~VCommandBuffer()
	{
		vkFreeCommandBuffers(
			this->device_manager->GetLogicalDevice(),
			this->owning_pool->GetNativeHandle(),
			1,
			&this->native_handle
		);
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

		lambda(this);

		this->EndCmdBuffer();
		this->GraphicsQueueSubmit();

		vkQueueWaitIdle(this->device_manager->GetGraphicsQueue());
	}

	void VCommandBuffer::GraphicsQueueSubmit()
	{
		VkSubmitInfo submit_info{};
		submit_info.sType			   = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers	   = &this->native_handle;

		vkQueueSubmit(this->device_manager->GetGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE);
	}

} // namespace Engine::Rendering::Vulkan
