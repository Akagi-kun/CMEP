#include "Rendering/Vulkan/Wrappers/Buffer.hpp"

#include "Rendering/Vulkan/Wrappers/Instance.hpp"
#include "Rendering/Vulkan/Wrappers/InstanceOwned.hpp"
#include "Rendering/Vulkan/Wrappers/LogicalDevice.hpp"

#include "vulkan/vulkan_core.h"

#include <stdexcept>

namespace Engine::Rendering::Vulkan
{
	Buffer::Buffer(
		InstanceOwned::value_t with_instance,
		VkDeviceSize with_size,
		VkBufferUsageFlags with_usage,
		VkMemoryPropertyFlags with_properties,
		VmaAllocationCreateFlags with_vma_alloc_flags
	)
		: InstanceOwned(with_instance), HoldsVMA(with_instance->GetGraphicMemoryAllocator()), buffer_size(with_size)
	{
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

		if (vmaCreateBuffer(
				*this->allocator,
				&buffer_info,
				&vma_alloc_info,
				&(this->native_handle),
				&(this->allocation),
				&(this->allocation_info)
			) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create buffer!");
		}

		vmaSetAllocationName(*this->allocator, this->allocation, "VBuffer");
	}

	Buffer::~Buffer()
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		logical_device->WaitDeviceIdle();

		vkDestroyBuffer(*logical_device, this->native_handle, nullptr);

		vmaFreeMemory(*this->allocator, this->allocation);
	}

	void Buffer::MapMemory()
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		vkMapMemory(
			*logical_device,
			this->allocation_info.deviceMemory,
			this->allocation_info.offset,
			this->allocation_info.size,
			0,
			&this->mapped_data
		);
	}

	void Buffer::UnmapMemory()
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		vkUnmapMemory(*logical_device, this->allocation_info.deviceMemory);

		// ensure mapped_data is never non-null when not mapped
		this->mapped_data = nullptr;
	}
} // namespace Engine::Rendering::Vulkan
