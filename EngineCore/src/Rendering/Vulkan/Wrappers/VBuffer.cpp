#include "Rendering/Vulkan/Wrappers/VBuffer.hpp"

#include "Rendering/Vulkan/VDeviceManager.hpp"

#include "vulkan/vulkan_core.h"

#include <stdexcept>

namespace Engine::Rendering::Vulkan
{
	VBuffer::VBuffer(
		VDeviceManager* const with_device_manager,
		VkDeviceSize with_size,
		VkBufferUsageFlags with_usage,
		VkMemoryPropertyFlags with_properties,
		VmaAllocationCreateFlags with_vma_alloc_flags
	)
		: HoldsVulkanDevice(with_device_manager), HoldsVMA(with_device_manager->GetVmaAllocator()),
		  buffer_size(with_size)
	{

		// Create a buffer handle
		VkBufferCreateInfo buffer_info{};
		buffer_info.sType		= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size		= this->buffer_size;
		buffer_info.usage		= with_usage;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo vma_alloc_info = {};
		vma_alloc_info.usage				   = VMA_MEMORY_USAGE_AUTO;
		// VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		vma_alloc_info.flags				   = with_vma_alloc_flags;
		vma_alloc_info.requiredFlags		   = with_properties;

		// if (vkCreateBuffer(this->vkLogicalDevice, &bufferInfo, nullptr, &(new_buffer->buffer)) != VK_SUCCESS)
		if (vmaCreateBuffer(
				this->allocator,
				&buffer_info,
				&vma_alloc_info,
				&(this->native_handle),
				&(this->allocation),
				&(this->allocation_info)
			) != VK_SUCCESS)
		{
			// this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan failed creating buffer");
			throw std::runtime_error("failed to create buffer!");
		}

		vmaSetAllocationName(this->allocator, this->allocation, "VBuffer");
	}

	VBuffer::~VBuffer()
	{
		vkDestroyBuffer(this->device_manager->GetLogicalDevice(), this->native_handle, nullptr);
		vmaFreeMemory(this->allocator, this->allocation);
	}

	void VBuffer::MapMemory()
	{
		vkMapMemory(
			this->device_manager->GetLogicalDevice(),
			this->allocation_info.deviceMemory,
			this->allocation_info.offset,
			this->allocation_info.size,
			0,
			&this->mapped_data
		);
	}

	void VBuffer::UnmapMemory()
	{
		vkUnmapMemory(this->device_manager->GetLogicalDevice(), this->allocation_info.deviceMemory);

		// ensure mapped_data is never non-null when not mapped
		this->mapped_data = nullptr;
	}
} // namespace Engine::Rendering::Vulkan
