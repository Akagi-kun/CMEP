#pragma once

#include "Rendering/Vulkan/ImportVulkan.hpp"

#include "HoldsVMA.hpp"
#include "HoldsVulkanDevice.hpp"
#include "framework.hpp"
#include "vulkan/vulkan_core.h"

#include <cassert>

namespace Engine::Rendering::Vulkan
{
	class Image : public HoldsVulkanDevice, public HoldsVMA
	{
	private:
		VmaAllocationInfo allocation_info;
		VmaAllocation allocation;

	protected:
		VkImageLayout current_layout   = VK_IMAGE_LAYOUT_UNDEFINED;
		VkFormat image_format		   = VK_FORMAT_UNDEFINED;
		VkImage native_handle		   = VK_NULL_HANDLE;
		VkImageView native_view_handle = VK_NULL_HANDLE;

		ImageSize size{};

	public:
		Image(
			DeviceManager* with_device_manager,
			ImageSize with_size,
			VkSampleCountFlagBits num_samples,
			VkFormat format,
			VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VkImageTiling with_tiling		 = VK_IMAGE_TILING_OPTIMAL
		);
		~Image();

		void TransitionImageLayout(VkImageLayout new_layout);

		void AddImageView(VkImageAspectFlags with_aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT);

		[[nodiscard]] VkImage& GetNativeHandle()
		{
			assert(this->native_handle != VK_NULL_HANDLE && "This command pool has no valid native handle!");

			return this->native_handle;
		}

		[[nodiscard]] VkImageView& GetNativeViewHandle()
		{
			return this->native_view_handle;
		}

		[[nodiscard]] ImageSize GetSize() const noexcept
		{
			return this->size;
		}
	};
} // namespace Engine::Rendering::Vulkan
