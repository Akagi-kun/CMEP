#pragma once

#include "HandleWrapper.hpp"
#include "HoldsVMA.hpp"
#include "ImportVulkan.hpp"
#include "InstanceOwned.hpp"
#include "framework.hpp"
#include "vulkan/vulkan_enums.hpp"

#include <cassert>

namespace Engine::Rendering::Vulkan
{
	class Image : public InstanceOwned, public HoldsVMA, public HandleWrapper<vk::Image>
	{
	public:
		Image(
			InstanceOwned::value_t with_instance,
			ImageSize with_size,
			vk::SampleCountFlagBits num_samples,
			vk::Format format,
			vk::ImageUsageFlags usage,
			VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VkImageTiling with_tiling		 = VK_IMAGE_TILING_OPTIMAL
		);
		~Image();

		void TransitionImageLayout(vk::ImageLayout new_layout);

		void AddImageView(vk::ImageAspectFlags with_aspect_flags = vk::ImageAspectFlagBits::eColor);

		[[nodiscard]] vk::ImageView& GetNativeViewHandle()
		{
			return native_view_handle;
		}

		[[nodiscard]] ImageSize GetSize() const
		{
			return size;
		}

	protected:
		vk::ImageLayout current_layout = vk::ImageLayout::eUndefined; // VK_IMAGE_LAYOUT_UNDEFINED;
		vk::Format image_format		   = vk::Format::eUndefined;	  // VK_FORMAT_UNDEFINED;
		vk::ImageView native_view_handle;
		// VK_NULL_HANDLE;

		ImageSize size{};

	private:
		VmaAllocationInfo allocation_info;
		VmaAllocation allocation;
	};
} // namespace Engine::Rendering::Vulkan
