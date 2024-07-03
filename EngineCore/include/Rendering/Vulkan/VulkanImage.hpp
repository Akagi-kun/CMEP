#pragma once

#include "Rendering/Vulkan/ImportVulkan.hpp"

#include "HoldsVMA.hpp"
#include "HoldsVulkanDevice.hpp"
#include "vulkan/vulkan_core.h"

namespace Engine::Rendering
{
	struct VulkanImageSize
	{
		uint32_t x;
		uint32_t y;
	};

	class VulkanCommandBuffer;

	class VulkanImage : public HoldsVulkanDevice, public HoldsVMA
	{
	protected:
		VmaAllocationInfo allocation_info;
		VmaAllocation allocation;

		VkImageLayout current_layout = VK_IMAGE_LAYOUT_UNDEFINED;

	public:
		VkImage image = VK_NULL_HANDLE;
		VkFormat image_format;

		VkImageView image_view = VK_NULL_HANDLE;

		VulkanImage(
			VulkanDeviceManager* with_device_manager,
			VmaAllocator with_allocator,
			VulkanImageSize size,
			VkSampleCountFlagBits num_samples,
			VkFormat format,
			VkImageTiling tiling,
			VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties
		);
		~VulkanImage();

		void TransitionImageLayout(VulkanCommandPool* with_pool, VkFormat format, VkImageLayout new_layout);

		void AddImageView(VkImageAspectFlags with_aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT);
	};
} // namespace Engine::Rendering
