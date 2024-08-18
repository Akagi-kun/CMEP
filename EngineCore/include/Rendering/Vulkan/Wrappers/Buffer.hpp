#pragma once

#include "Rendering/Vulkan/VulkanStructDefs.hpp"
#include "Rendering/Vulkan/Wrappers/HoldsVMA.hpp"
#include "Rendering/Vulkan/Wrappers/InstanceOwned.hpp"
#include "Rendering/Vulkan/Wrappers/framework.hpp"

#include "InstanceOwned.hpp"
#include "vulkan/vulkan_core.h"

#include <cstring>
#include <vector>

namespace Engine::Rendering::Vulkan
{
	class Buffer : public InstanceOwned, public HoldsVMA, public HandleWrapper<VkBuffer>
	{
	public:
		void* mapped_data = nullptr;

		Buffer(
			InstanceOwned::value_t with_instance,
			VkDeviceSize with_size,
			VkBufferUsageFlags with_usage,
			VkMemoryPropertyFlags with_properties,
			VmaAllocationCreateFlags with_vma_alloc_flags = 0
		);
		~Buffer();

		void MapMemory();
		void UnmapMemory();

		void MemoryCopy(const void* with_data, size_t with_size)
		{
			this->MapMemory();

			std::memcpy(this->mapped_data, with_data, with_size);

			this->UnmapMemory();
		}

	protected:
		VkDeviceSize buffer_size;

		VmaAllocation allocation;
		VmaAllocationInfo allocation_info;
	};

	class StagingBuffer final : public Buffer
	{
	public:
		StagingBuffer(InstanceOwned::value_t with_instance, const void* with_data, VkDeviceSize with_size);
	};

	class VertexBuffer final : public Buffer
	{
	public:
		VertexBuffer(InstanceOwned::value_t with_instance, const std::vector<RenderingVertex>& vertices);
	};
} // namespace Engine::Rendering::Vulkan
