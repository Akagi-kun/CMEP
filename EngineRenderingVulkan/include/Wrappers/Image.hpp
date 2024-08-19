#pragma once

#include "HandleWrapper.hpp"
#include "HoldsVMA.hpp"
#include "ImportVulkan.hpp"
#include "InstanceOwned.hpp"
#include "framework.hpp"

#include <cassert>

namespace Engine::Rendering::Vulkan
{
	class Image : public InstanceOwned, public HoldsVMA, public HandleWrapper<VkImage>
	{
	public:
		Image(
			InstanceOwned::value_t with_instance,
			ImageSize with_size,
			vk::SampleCountFlagBits num_samples,
			VkFormat format,
			VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VkImageTiling with_tiling		 = VK_IMAGE_TILING_OPTIMAL
		);
		~Image();

		void TransitionImageLayout(VkImageLayout new_layout);

		void AddImageView(VkImageAspectFlags with_aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT);

		[[nodiscard]] VkImageView& GetNativeViewHandle()
		{
			return native_view_handle;
		}

		[[nodiscard]] ImageSize GetSize() const
		{
			return size;
		}

	protected:
		VkImageLayout current_layout   = VK_IMAGE_LAYOUT_UNDEFINED;
		VkFormat image_format		   = VK_FORMAT_UNDEFINED;
		VkImageView native_view_handle = VK_NULL_HANDLE;

		ImageSize size{};

	private:
		VmaAllocationInfo allocation_info;
		VmaAllocation allocation;
	};
} // namespace Engine::Rendering::Vulkan
