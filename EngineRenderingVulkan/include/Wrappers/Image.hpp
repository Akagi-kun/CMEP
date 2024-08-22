#pragma once

#include "Rendering/Transform.hpp"

#include "HandleWrapper.hpp"
#include "HoldsVMA.hpp"
#include "ImportVulkan.hpp"
#include "InstanceOwned.hpp"
#include "framework.hpp"

#include <cassert>

namespace Engine::Rendering::Vulkan
{
	class Image : public InstanceOwned, public HoldsVMA, public HandleWrapper<vk::raii::Image>
	{
	public:
		Image(
			InstanceOwned::value_t with_instance,
			ImageSize with_size,
			vk::SampleCountFlagBits num_samples,
			vk::Format format,
			vk::ImageUsageFlags usage,
			vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eDeviceLocal,
			vk::ImageTiling with_tiling		   = vk::ImageTiling::eOptimal
		);
		~Image();

		void TransitionImageLayout(vk::ImageLayout new_layout);

		void AddImageView(vk::ImageAspectFlags with_aspect_flags = vk::ImageAspectFlagBits::eColor);

		[[nodiscard]] vk::raii::ImageView* GetNativeViewHandle()
		{
			return native_view_handle;
		}

		[[nodiscard]] ImageSize GetSize() const
		{
			return size;
		}

	protected:
		vk::ImageLayout current_layout = vk::ImageLayout::eUndefined;
		vk::Format image_format		   = vk::Format::eUndefined;
		vk::raii::ImageView* native_view_handle;

		ImageSize size{};

	private:
		VmaAllocationInfo allocation_info;
		VmaAllocation allocation;
	};
} // namespace Engine::Rendering::Vulkan
