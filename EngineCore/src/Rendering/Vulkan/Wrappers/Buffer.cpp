#include "Rendering/Vulkan/Wrappers/Buffer.hpp"

#include "Rendering/Vulkan/Wrappers/CommandBuffer.hpp"
#include "Rendering/Vulkan/Wrappers/CommandPool.hpp"
#include "Rendering/Vulkan/Wrappers/Instance.hpp"
#include "Rendering/Vulkan/Wrappers/InstanceOwned.hpp"
#include "Rendering/Vulkan/Wrappers/LogicalDevice.hpp"

#include "vulkan/vulkan_core.h"

#include <stdexcept>

namespace Engine::Rendering::Vulkan
{
#pragma region Public

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

	StagingBuffer::StagingBuffer(InstanceOwned::value_t with_instance, const void* with_data, VkDeviceSize with_size)
		: Buffer(
			  with_instance,
			  with_size,
			  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		  )
	{
		this->MemoryCopy(with_data, with_size);
	}

	VertexBuffer::VertexBuffer(InstanceOwned::value_t with_instance, const std::vector<RenderingVertex>& vertices)
		: Buffer(
			  with_instance,
			  sizeof(vertices[0]) * vertices.size(),
			  VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		  )
	{
		CommandBuffer* command_buffer = with_instance->GetCommandPool()->AllocateCommandBuffer();

		auto staging_buffer = StagingBuffer(with_instance, vertices.data(), buffer_size);

		// Copy into final buffer
		command_buffer->BufferBufferCopy(&staging_buffer, this, {VkBufferCopy{0, 0, buffer_size}});

		// delete staging_buffer;
		delete command_buffer;
	}

#pragma endregion
} // namespace Engine::Rendering::Vulkan
