#pragma once

#include "fwd.hpp"

#include "backend/MemoryAllocator.hpp"
#include "common/HoldsVMA.hpp"
#include "common/InstanceOwned.hpp"
#include "common/StructDefs.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <cassert>
#include <cstring>
#include <vector>

namespace Engine::Rendering::Vulkan
{
	/**
	 * @brief Class representing a %Vulkan buffer
	 */
	class Buffer : public HoldsVMA, public HandleWrapper<vk::raii::Buffer>
	{
	public:
		void* mapped_data = nullptr;

		/**
		 * @brief Construct a Vulkan Buffer object
		 *
		 * @param with_device Logical device to create the buffer on
		 * @param with_allocator Graphic memory allocator to create the buffer's memory with
		 * @param with_size Size of buffer memory
		 * @param with_usage Buffer usage
		 * @param with_properties Memory properties
		 */
		Buffer(
			const LogicalDevice*	with_device,
			MemoryAllocator*		with_allocator,
			vk::DeviceSize			with_size,
			vk::BufferUsageFlags	with_usage,
			vk::MemoryPropertyFlags with_properties
		);
		~Buffer();

		void mapMemory();
		void unmapMemory();

		/**
		 * @brief Copy a region of memory into this buffer
		 *
		 * @param with_data Pointer to the region to copy
		 * @param with_size Size of the region
		 */
		void memoryCopy(const void* with_data, size_t with_size)
		{
			assert(
				(with_size <= buffer_size) &&
				"Cannot copy a region larger than the buffer being copied into!"
			);
			assert(
				(property_flags & vk::MemoryPropertyFlagBits::eHostVisible) &&
				(property_flags & vk::MemoryPropertyFlagBits::eHostCoherent) &&
				"Tried host-copying into a buffer that is either not host visible or "
				"not host coherent!"
			);

			mapMemory();

			std::memcpy(mapped_data, with_data, with_size);

			unmapMemory();
		}

	protected:
		const LogicalDevice* device;

		const vk::DeviceSize buffer_size;

		const vk::MemoryPropertyFlags property_flags;

		VmaAllocation	  allocation;
		VmaAllocationInfo allocation_info;
	};

#pragma region Specializations

	/**
	 * @brief Specializes @ref Buffer for staging purposes
	 */
	class StagingBuffer final : public Buffer
	{
	public:
		/**
		 * @brief Construct in-place with data
		 *
		 * @param with_device,with_allocator See Buffer for common parameters
		 * @param with_data The data used to create this buffer
		 * @param with_size Size of data
		 */
		StagingBuffer(
			LogicalDevice*	 with_device,
			MemoryAllocator* with_allocator,
			const void*		 with_data,
			vk::DeviceSize	 with_size
		);
	};

	/**
	 * @brief Specializes @ref Buffer to hold @ref RenderingVertex "vertices"
	 */
	class VertexBuffer final : public Buffer
	{
	public:
		/**
		 * @brief Construct in-place with data
		 *
		 * @param with_device,with_allocator See Buffer for common parameters
		 * @param with_commandbuffer The command buffer to use when copying data
		 * @param vertices A container of vertices to copy into this buffer
		 */
		VertexBuffer(
			LogicalDevice*						with_device,
			MemoryAllocator*					with_allocator,
			CommandBuffer&						with_commandbuffer,
			const std::vector<RenderingVertex>& vertices
		);
	};

	/**
	 * @brief Specializes @ref Buffer to hold uniform data
	 */
	class UniformBuffer final : public Buffer
	{
	public:
		/**
		 * @brief Construct with size
		 *
		 * @param with_device,with_allocator See Buffer for common parameters
		 * @param with_size Size of the buffer
		 */
		UniformBuffer(
			LogicalDevice*	 with_device,
			MemoryAllocator* with_allocator,
			vk::DeviceSize	 with_size
		);
	};

#pragma endregion
} // namespace Engine::Rendering::Vulkan
