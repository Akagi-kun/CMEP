#include "Wrappers/Buffer.hpp"

#include "ImportVulkan.hpp"
#include "Wrappers/CommandBuffer.hpp"
#include "Wrappers/CommandPool.hpp"
#include "Wrappers/Instance.hpp"
#include "Wrappers/InstanceOwned.hpp"
#include "Wrappers/LogicalDevice.hpp"

#include <stdexcept>

namespace Engine::Rendering::Vulkan
{
#pragma region Public

	Buffer::Buffer(
		InstanceOwned::value_t with_instance,
		vk::DeviceSize with_size,
		vk::BufferUsageFlags with_usage,
		vk::MemoryPropertyFlags with_properties
	)
		: InstanceOwned(with_instance), HoldsVMA(with_instance->GetGraphicMemoryAllocator()), buffer_size(with_size)
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		vk::BufferCreateInfo create_info({}, buffer_size, with_usage, vk::SharingMode::eExclusive, {}, {}, {});

		VmaAllocationCreateInfo vma_alloc_info = {};
		vma_alloc_info.usage				   = VMA_MEMORY_USAGE_UNKNOWN;
		vma_alloc_info.flags				   = 0;
		vma_alloc_info.requiredFlags		   = static_cast<VkMemoryPropertyFlags>(with_properties);

		native_handle = logical_device->GetHandle().createBuffer(create_info);

		if (vmaAllocateMemoryForBuffer(*allocator, native_handle, &vma_alloc_info, &allocation, &allocation_info) !=
			VK_SUCCESS)
		{
			throw std::runtime_error("Could not allocate buffer memory!");
		}

		if (vmaBindBufferMemory(*allocator, allocation, native_handle) != VK_SUCCESS)
		{
			throw std::runtime_error("Could not bind buffer memory!");
		}

		vmaSetAllocationName(*allocator, allocation, "Buffer");
	}

	Buffer::~Buffer()
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		logical_device->GetHandle().waitIdle();

		logical_device->GetHandle().destroyBuffer(native_handle);

		vmaFreeMemory(*allocator, allocation);
	}

	void Buffer::MapMemory()
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		mapped_data = logical_device->GetHandle()
						  .mapMemory(allocation_info.deviceMemory, allocation_info.offset, allocation_info.size);
		/* vkMapMemory(
			logical_device->GetHandle(),
			allocation_info.deviceMemory,
			allocation_info.offset,
			allocation_info.size,
			0,
			&mapped_data
		); */
	}

	void Buffer::UnmapMemory()
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		logical_device->GetHandle().unmapMemory(allocation_info.deviceMemory);
		// vkUnmapMemory(logical_device->GetHandle(), allocation_info.deviceMemory);

		// ensure mapped_data is never non-null when not mapped
		mapped_data = nullptr;
	}

	StagingBuffer::StagingBuffer(InstanceOwned::value_t with_instance, const void* with_data, vk::DeviceSize with_size)
		: Buffer(
			  with_instance,
			  with_size,
			  vk::BufferUsageFlagBits::eTransferSrc,
			  vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		  )
	{
		MemoryCopy(with_data, with_size);
	}

	VertexBuffer::VertexBuffer(InstanceOwned::value_t with_instance, const std::vector<RenderingVertex>& vertices)
		: Buffer(
			  with_instance,
			  sizeof(vertices[0]) * vertices.size(),
			  vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
			  vk::MemoryPropertyFlagBits::eDeviceLocal
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
