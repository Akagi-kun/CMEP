#pragma once

#include "fwd.hpp"

#include "backend/MemoryAllocator.hpp"
#include "common/HoldsVMA.hpp"
#include "common/InstanceOwned.hpp"
#include "common/StructDefs.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <cstring>
#include <vector>

namespace Engine::Rendering::Vulkan
{
	class Buffer : public InstanceOwned,
				   public HoldsVMA,
				   public HandleWrapper<vk::raii::Buffer>
	{
	public:
		void* mapped_data = nullptr;

		Buffer(
			InstanceOwned::value_t	with_instance,
			vk::DeviceSize			with_size,
			vk::BufferUsageFlags	with_usage,
			vk::MemoryPropertyFlags with_properties
		);
		~Buffer();

		void mapMemory();
		void unmapMemory();

		void memoryCopy(const void* with_data, size_t with_size)
		{
			mapMemory();

			std::memcpy(mapped_data, with_data, with_size);

			unmapMemory();
		}

	protected:
		vk::DeviceSize buffer_size;

		VmaAllocation	  allocation;
		VmaAllocationInfo allocation_info;
	};

	// Specializations of Buffer
	class StagingBuffer final : public Buffer
	{
	public:
		StagingBuffer(
			InstanceOwned::value_t with_instance,
			const void*			   with_data,
			vk::DeviceSize		   with_size
		);
	};

	class VertexBuffer final : public Buffer
	{
	public:
		VertexBuffer(
			InstanceOwned::value_t				with_instance,
			const std::vector<RenderingVertex>& vertices
		);
	};

	class UniformBuffer final : public Buffer
	{
	public:
		UniformBuffer(InstanceOwned::value_t, vk::DeviceSize with_size);
	};
} // namespace Engine::Rendering::Vulkan
