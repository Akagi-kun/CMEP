#include "objects/Image.hpp"

#include "fwd.hpp"

#include "Rendering/Transform.hpp"

#include "backend/Instance.hpp"
#include "common/Utility.hpp"
#include "objects/CommandBuffer.hpp"
#include "objects/CommandPool.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <stdexcept>

namespace Engine::Rendering::Vulkan
{
	Image::Image(
		InstanceOwned::value_t	with_instance,
		ImageSize				with_size,
		vk::SampleCountFlagBits num_samples,
		vk::Format				format,
		vk::ImageUsageFlags		usage,
		vk::MemoryPropertyFlags properties,
		vk::ImageTiling			tiling
	)
		: InstanceOwned(with_instance),
		  HoldsVMA(with_instance->getGraphicMemoryAllocator()), image_format(format),
		  size(with_size)
	{
		LogicalDevice* logical_device = instance->getLogicalDevice();

		vk::ImageCreateInfo create_info(
			{},
			vk::ImageType::e2D,
			image_format,
			Utility::convertToExtent<vk::Extent3D>(size, 1),
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

		native_handle = logical_device->createImage(create_info);

		if (vmaAllocateMemoryForImage(
				allocator->getHandle(),
				*native_handle,
				&vma_alloc_info,
				&allocation,
				&allocation_info
			) != VK_SUCCESS)
		{
			throw std::runtime_error("Could not allocate image memory!");
		}

		if (vmaBindImageMemory(allocator->getHandle(), allocation, *native_handle) !=
			VK_SUCCESS)
		{
			throw std::runtime_error("Could not bind image memory!");
		}

		vmaSetAllocationName(allocator->getHandle(), allocation, "Image");
	}

	Image::~Image()
	{
		// Ensure image is released before memory is deallocated
		native_handle.clear();
		vmaFreeMemory(allocator->getHandle(), allocation);
	}

	void Image::transitionImageLayout(vk::ImageLayout new_layout)
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

		if (current_layout == vk::ImageLayout::eUndefined &&
			new_layout == vk::ImageLayout::eTransferDstOptimal)
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

		auto command_buffer = instance->getCommandPool()->constructCommandBuffer();

		command_buffer.recordCmds([&](vk::raii::CommandBuffer* with_buffer) {
			with_buffer->pipelineBarrier(src_stage, dst_stage, {}, {}, {}, barrier);
		});

		current_layout = new_layout;
	}

	ViewedImage::ViewedImage(
		InstanceOwned::value_t	with_instance,
		ImageSize				with_size,
		vk::SampleCountFlagBits num_samples,
		vk::Format				format,
		vk::ImageUsageFlags		usage,
		vk::ImageAspectFlags	with_aspect_flags,
		vk::MemoryPropertyFlags properties,
		vk::ImageTiling			with_tiling
	)
		: Image(with_instance, with_size, num_samples, format, usage, properties, with_tiling)
	{
		LogicalDevice* logical_device = instance->getLogicalDevice();

		vk::ImageViewCreateInfo view_create_info(
			{},
			*native_handle,
			vk::ImageViewType::e2D,
			image_format,
			{},
			{with_aspect_flags, 0, 1, 0, 1}
		);

		view_handle = logical_device->createImageView(view_create_info);
	}

} // namespace Engine::Rendering::Vulkan
