#include "objects/Buffer.hpp"

#include "backend/Instance.hpp"
#include "backend/LogicalDevice.hpp"
#include "common/InstanceOwned.hpp"
#include "objects/CommandBuffer.hpp"
#include "objects/CommandPool.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <stdexcept>
#include <vector>

namespace Engine::Rendering::Vulkan
{
#pragma region Public

	Buffer::Buffer(
		InstanceOwned::value_t	with_instance,
		vk::DeviceSize			with_size,
		vk::BufferUsageFlags	with_usage,
		vk::MemoryPropertyFlags with_properties
	)
		: InstanceOwned(with_instance),
		  HoldsVMA(with_instance->getGraphicMemoryAllocator()), buffer_size(with_size)
	{
		LogicalDevice* logical_device = instance->getLogicalDevice();

		vk::BufferCreateInfo create_info{
			.size				   = buffer_size,
			.usage				   = with_usage,
			.sharingMode		   = vk::SharingMode::eExclusive,
			.queueFamilyIndexCount = {},
			.pQueueFamilyIndices   = {}
		};

		VmaAllocationCreateInfo vma_alloc_info = {};
		vma_alloc_info.usage				   = VMA_MEMORY_USAGE_UNKNOWN;
		vma_alloc_info.flags				   = 0;
		vma_alloc_info.requiredFlags = static_cast<VkMemoryPropertyFlags>(with_properties);

		native_handle = logical_device->createBuffer(create_info);

		if (vmaAllocateMemoryForBuffer(
				allocator->getHandle(),
				*native_handle,
				&vma_alloc_info,
				&allocation,
				&allocation_info
			) != VK_SUCCESS)
		{
			throw std::runtime_error("Could not allocate buffer memory!");
		}

		if (vmaBindBufferMemory(allocator->getHandle(), allocation, *native_handle) !=
			VK_SUCCESS)
		{
			throw std::runtime_error("Could not bind buffer memory!");
		}

		vmaSetAllocationName(allocator->getHandle(), allocation, "Buffer");
	}

	Buffer::~Buffer()
	{
		LogicalDevice* logical_device = instance->getLogicalDevice();

		logical_device->waitIdle();

		vmaFreeMemory(allocator->getHandle(), allocation);
	}

	void Buffer::mapMemory()
	{
		LogicalDevice* logical_device = instance->getLogicalDevice();

		mapped_data = (**logical_device)
						  .mapMemory(
							  allocation_info.deviceMemory,
							  allocation_info.offset,
							  allocation_info.size
						  );
	}

	void Buffer::unmapMemory()
	{
		LogicalDevice* logical_device = instance->getLogicalDevice();

		(**logical_device).unmapMemory(allocation_info.deviceMemory);

		// ensure mapped_data is never non-null when not mapped
		mapped_data = nullptr;
	}

	StagingBuffer::StagingBuffer(
		InstanceOwned::value_t with_instance,
		const void*			   with_data,
		vk::DeviceSize		   with_size
	)
		: Buffer(
			  with_instance,
			  with_size,
			  vk::BufferUsageFlagBits::eTransferSrc,
			  vk::MemoryPropertyFlagBits::eHostVisible |
				  vk::MemoryPropertyFlagBits::eHostCoherent
		  )
	{
		memoryCopy(with_data, with_size);
	}

	VertexBuffer::VertexBuffer(
		InstanceOwned::value_t				with_instance,
		const std::vector<RenderingVertex>& vertices
	)
		: Buffer(
			  with_instance,
			  sizeof(vertices[0]) * vertices.size(),
			  vk::BufferUsageFlagBits::eTransferDst |
				  vk::BufferUsageFlagBits::eVertexBuffer,
			  vk::MemoryPropertyFlagBits::eDeviceLocal
		  )
	{
		CommandBuffer* command_buffer =
			with_instance->getCommandPool()->allocateCommandBuffer();

		auto staging_buffer = StagingBuffer(with_instance, vertices.data(), buffer_size);

		// Copy into final buffer
		command_buffer->copyBufferBuffer(
			with_instance->getLogicalDevice()->getGraphicsQueue(),
			&staging_buffer,
			this,
			{vk::BufferCopy{0, 0, buffer_size}}
		);

		delete command_buffer;
	}

	UniformBuffer::UniformBuffer(InstanceOwned::value_t with_instance, vk::DeviceSize with_size)
		: Buffer(
			  with_instance,
			  with_size,
			  vk::BufferUsageFlagBits::eUniformBuffer,
			  vk::MemoryPropertyFlagBits::eHostVisible |
				  vk::MemoryPropertyFlagBits::eHostCoherent
		  )
	{}

#pragma endregion
} // namespace Engine::Rendering::Vulkan
