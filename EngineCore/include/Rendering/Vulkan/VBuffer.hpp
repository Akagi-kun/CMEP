#pragma once

#include "Rendering/Vulkan/HoldsVMA.hpp"
#include "Rendering/Vulkan/HoldsVulkanDevice.hpp"

#include "framework.hpp"
#include "vulkan/vulkan_core.h"

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
			VulkanDeviceManager* with_device_manager,
			VmaAllocator with_allocator,
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

		[[nodiscard]] VkBuffer& GetNativeHandle()
		{
			return this->native_handle;
		}
	};
} // namespace Engine::Rendering::Vulkan
