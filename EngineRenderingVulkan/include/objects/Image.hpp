#pragma once

#include "fwd.hpp"

#include "Rendering/Transform.hpp"

#include "backend/MemoryAllocator.hpp"
#include "common/HandleWrapper.hpp"
#include "common/HoldsVMA.hpp"
#include "common/InstanceOwned.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <cassert>

namespace Engine::Rendering::Vulkan
{
	class Image : public InstanceOwned,
				  public HoldsVMA,
				  public HandleWrapper<vk::raii::Image>
	{
	public:
		Image(
			InstanceOwned::value_t	with_instance,
			ImageSize				with_size,
			vk::SampleCountFlagBits num_samples,
			vk::Format				format,
			vk::ImageUsageFlags		usage,
			vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eDeviceLocal,
			vk::ImageTiling			with_tiling = vk::ImageTiling::eOptimal
		);
		~Image();

		void transitionImageLayout(vk::ImageLayout new_layout);

		[[nodiscard]] ImageSize getSize() const
		{
			return size;
		}

	protected:
		vk::ImageLayout current_layout = vk::ImageLayout::eUndefined;
		vk::Format		image_format   = vk::Format::eUndefined;

		ImageSize size{};

	private:
		VmaAllocationInfo allocation_info;
		VmaAllocation	  allocation;
	};

	class ViewedImage : public Image
	{
	public:
		ViewedImage(
			InstanceOwned::value_t	with_instance,
			ImageSize				with_size,
			vk::SampleCountFlagBits num_samples,
			vk::Format				format,
			vk::ImageUsageFlags		usage,
			vk::ImageAspectFlags	with_aspect_flags,
			vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eDeviceLocal,
			vk::ImageTiling			with_tiling = vk::ImageTiling::eOptimal
		);

		[[nodiscard]] vk::raii::ImageView& getNativeViewHandle()
		{
			return view_handle;
		}

	protected:
		vk::raii::ImageView view_handle = nullptr;
	};
} // namespace Engine::Rendering::Vulkan
