#include "objects/Image.hpp"

#include "fwd.hpp"

#include "ImportVulkan.hpp"
#include "backend/Instance.hpp"
#include "common/Utilities.hpp"
#include "objects/CommandBuffer.hpp"
#include "objects/CommandPool.hpp"

#include <stdexcept>

namespace Engine::Rendering::Vulkan
{
	Image::Image(
		InstanceOwned::value_t with_instance,
		ImageSize with_size,
		vk::SampleCountFlagBits num_samples,
		vk::Format format,
		vk::ImageUsageFlags usage,
		vk::MemoryPropertyFlags properties,
		vk::ImageTiling tiling
	)
		: InstanceOwned(with_instance), HoldsVMA(with_instance->GetGraphicMemoryAllocator()), image_format(format),
		  size(with_size)
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		vk::ImageCreateInfo create_info(
			{},
			vk::ImageType::e2D,
			image_format,
			Utility::ConvertToExtent<vk::Extent3D>(size, 1),
			1,
			1,
			num_samples,
			tiling,
			usage,
			vk::SharingMode::eExclusive,
			{},
			{},
			vk::ImageLayout::eUndefined,
			{}
		);

		VmaAllocationCreateInfo vma_alloc_info{};
		vma_alloc_info.usage		 = VMA_MEMORY_USAGE_UNKNOWN;
		vma_alloc_info.flags		 = 0;
		vma_alloc_info.requiredFlags = static_cast<VkMemoryPropertyFlags>(properties);

		native_handle = logical_device->GetHandle().createImage(create_info);

		if (vmaAllocateMemoryForImage(
				allocator->GetHandle(),
				*native_handle,
				&vma_alloc_info,
				&allocation,
				&allocation_info
			) != VK_SUCCESS)
		{
			throw std::runtime_error("Could not allocate image memory!");
		}

		if (vmaBindImageMemory(allocator->GetHandle(), allocation, *native_handle) != VK_SUCCESS)
		{
			throw std::runtime_error("Could not bind image memory!");
		}

		vmaSetAllocationName(allocator->GetHandle(), allocation, "Image");
	}

	Image::~Image()
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		logical_device->GetHandle().waitIdle();

		// delete native_view_handle;

		// Ensure image is released before memory is deallocated
		native_handle.clear();
		vmaFreeMemory(allocator->GetHandle(), allocation);
	}

	void Image::TransitionImageLayout(vk::ImageLayout new_layout)
	{
		vk::ImageMemoryBarrier barrier(
			{},
			{},
			current_layout,
			new_layout,
			vk::QueueFamilyIgnored,
			vk::QueueFamilyIgnored,
			*native_handle,
			{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1},
			{}
		);

		vk::PipelineStageFlags src_stage;
		vk::PipelineStageFlags dst_stage;

		if (current_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eTransferDstOptimal)
		{
			barrier.srcAccessMask = {};
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

			src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
			dst_stage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (current_layout == vk::ImageLayout::eTransferDstOptimal &&
				 new_layout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			src_stage = vk::PipelineStageFlagBits::eTransfer;
			dst_stage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else
		{
			throw std::invalid_argument("Unsupported layout transition!");
		}

		auto* command_buffer = instance->GetCommandPool()->AllocateCommandBuffer();

		command_buffer->RecordCmds([&](CommandBuffer* with_buffer) {
			with_buffer->GetHandle().pipelineBarrier(src_stage, dst_stage, {}, {}, {}, barrier);
		});

		delete command_buffer;

		this->current_layout = new_layout;
	}

	ViewedImage::ViewedImage(
		InstanceOwned::value_t with_instance,
		ImageSize with_size,
		vk::SampleCountFlagBits num_samples,
		vk::Format format,
		vk::ImageUsageFlags usage,
		vk::ImageAspectFlags with_aspect_flags,
		vk::MemoryPropertyFlags properties,
		vk::ImageTiling with_tiling
	)
		: Image(with_instance, with_size, num_samples, format, usage, properties, with_tiling)
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		vk::ImageViewCreateInfo view_create_info(
			{},
			*native_handle,
			vk::ImageViewType::e2D,
			image_format,
			{},
			{with_aspect_flags, 0, 1, 0, 1}
		);

		native_view_handle = logical_device->GetHandle().createImageView(view_create_info);
	}

	/* void ViewedImage::AddImageView(vk::ImageAspectFlags with_aspect_flags)
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		vk::ImageViewCreateInfo view_create_info(
			{},
			*native_handle,
			vk::ImageViewType::e2D,
			image_format,
			{},
			{with_aspect_flags, 0, 1, 0, 1}
		);

		native_view_handle = logical_device->GetHandle().createImageView(view_create_info);
	} */
} // namespace Engine::Rendering::Vulkan
