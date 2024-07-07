#pragma once

#include "Rendering/Vulkan/ImportVulkan.hpp"

#include "HoldsVMA.hpp"
#include "HoldsVulkanDevice.hpp"
#include "framework.hpp"
#include "vulkan/vulkan_core.h"

#include <cassert>

namespace Engine::Rendering::Vulkan
{
	class VImage : public HoldsVulkanDevice, public HoldsVMA
	{
	private:
		VmaAllocationInfo allocation_info;
		VmaAllocation allocation;

	protected:
		VkImageLayout current_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkFormat image_format		 = VK_FORMAT_UNDEFINED;
		VkImage native_handle		 = VK_NULL_HANDLE;

	public:
		// TODO: Make protected
		VkImageView image_view = VK_NULL_HANDLE;

		VImage(
			VulkanDeviceManager* with_device_manager,
			VmaAllocator with_allocator,
			VulkanImageSize size,
			VkSampleCountFlagBits num_samples,
			VkFormat format,
			VkImageTiling tiling,
			VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties
		);
		~VImage();

		void TransitionImageLayout(VCommandPool* with_pool, VkFormat format, VkImageLayout new_layout);

		void AddImageView(VkImageAspectFlags with_aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT);

		[[nodiscard]] VkImage& GetNativeHandle()
		{
			assert(this->native_handle != VK_NULL_HANDLE && "This command pool has no valid native handle!");

			return this->native_handle;
		}
	};
} // namespace Engine::Rendering::Vulkan
