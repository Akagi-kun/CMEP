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
		const LogicalDevice*	with_device,
		MemoryAllocator*		with_allocator,
		vk::DeviceSize			with_size,
		vk::BufferUsageFlags	with_usage,
		vk::MemoryPropertyFlags with_properties
	)
		: HoldsVMA(with_allocator), device(with_device), buffer_size(with_size),
		  property_flags(with_properties)
	{
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

		native_handle = device->createBuffer(create_info);

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
		device->waitIdle();

		vmaFreeMemory(allocator->getHandle(), allocation);
	}

	void Buffer::mapMemory()
	{
		mapped_data = static_cast<vk::Device>(*device).mapMemory(
			allocation_info.deviceMemory,
			allocation_info.offset,
			allocation_info.size
		);
	}

	void Buffer::unmapMemory()
	{
		static_cast<vk::Device>(*device).unmapMemory(allocation_info.deviceMemory);

		// ensure mapped_data is never non-null when not mapped
		mapped_data = nullptr;
	}

	StagingBuffer::StagingBuffer(
		LogicalDevice*	 with_device,
		MemoryAllocator* with_allocator,
		const void*		 with_data,
		vk::DeviceSize	 with_size
	)
		: Buffer(
			  with_device,
			  with_allocator,
			  with_size,
			  vk::BufferUsageFlagBits::eTransferSrc,
			  vk::MemoryPropertyFlagBits::eHostVisible |
				  vk::MemoryPropertyFlagBits::eHostCoherent
		  )
	{
		memoryCopy(with_data, with_size);
	}

	VertexBuffer::VertexBuffer(
		LogicalDevice*						with_device,
		MemoryAllocator*					with_allocator,
		CommandBuffer&						with_commandbuffer,
		const std::vector<RenderingVertex>& vertices
	)
		: Buffer(
			  with_device,
			  with_allocator,
			  sizeof(vertices[0]) * vertices.size(),
			  vk::BufferUsageFlagBits::eTransferDst |
				  vk::BufferUsageFlagBits::eVertexBuffer,
			  vk::MemoryPropertyFlagBits::eDeviceLocal
		  )
	{
		auto staging_buffer =
			StagingBuffer(with_device, with_allocator, vertices.data(), buffer_size);

		// Copy into final buffer
		with_commandbuffer.copyBufferBuffer(
			with_device->getGraphicsQueue(),
			&staging_buffer,
			this,
			{vk::BufferCopy{0, 0, buffer_size}}
		);
	}

	UniformBuffer::UniformBuffer(
		LogicalDevice*	 with_device,
		MemoryAllocator* with_allocator,
		vk::DeviceSize	 with_size
	)
		: Buffer(
			  with_device,
			  with_allocator,
			  with_size,
			  vk::BufferUsageFlagBits::eUniformBuffer,
			  vk::MemoryPropertyFlagBits::eHostVisible |
				  vk::MemoryPropertyFlagBits::eHostCoherent
		  )
	{}

#pragma endregion
} // namespace Engine::Rendering::Vulkan
