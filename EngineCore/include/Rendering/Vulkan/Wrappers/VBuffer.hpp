#pragma once

#include "Rendering/Vulkan/Wrappers/HoldsVMA.hpp"
#include "Rendering/Vulkan/Wrappers/HoldsVulkanDevice.hpp"
#include "Rendering/Vulkan/Wrappers/framework.hpp"

#include "vulkan/vulkan_core.h"

#include <cstring>

namespace Engine::Rendering::Vulkan
{
	class VBuffer : public HoldsVulkanDevice, public HoldsVMA
	{
	protected:
		VkBuffer native_handle;
		VkDeviceSize buffer_size;

		VmaAllocation allocation;
		VmaAllocationInfo allocation_info;

	public:
		void* mapped_data = nullptr;

		VBuffer(
			VDeviceManager* with_device_manager,
			VkDeviceSize with_size,
			VkBufferUsageFlags with_usage,
			VkMemoryPropertyFlags with_properties,
			VmaAllocationCreateFlags with_vma_alloc_flags
		);
		~VBuffer();

		void MapMemory();
		void UnmapMemory();

		void BufferCopy(
			VBuffer* with_src,
			VkDeviceSize with_region_size,
			VkDeviceSize with_src_offset = 0,
			VkDeviceSize with_dst_offset = 0
		);

		void MemoryCopy(const void* with_data, size_t with_size)
		{
			this->MapMemory();

			std::memcpy(this->mapped_data, with_data, with_size);

			this->UnmapMemory();
		}

		[[nodiscard]] VkBuffer& GetNativeHandle()
		{
			return this->native_handle;
		}
	};
} // namespace Engine::Rendering::Vulkan
