#include "objects/Image.hpp"

#include "fwd.hpp"

#include "Rendering/Transform.hpp"

#include "Exception.hpp"
#include "backend/Instance.hpp"
#include "common/Utility.hpp"
#include "objects/CommandBuffer.hpp"
#include "objects/CommandPool.hpp"
#include "vulkan/vulkan_raii.hpp"

namespace Engine::Rendering::Vulkan
{
	Image::Image(
		LogicalDevice*			with_device,
		MemoryAllocator*		with_allocator,
		ImageSize				with_size,
		vk::SampleCountFlagBits num_samples,
		vk::Format				format,
		vk::ImageUsageFlags		usage,
		vk::MemoryPropertyFlags properties,
		vk::ImageTiling			tiling
	)
		: HoldsVMA(with_allocator), device(with_device), image_format(format),
		  size(with_size)
	{
		vk::ImageCreateInfo create_info{
			.imageType	   = vk::ImageType::e2D,
			.format		   = image_format,
			.extent		   = Utility::convertToExtent<vk::Extent3D>(size, 1),
			.mipLevels	   = 1,
			.arrayLayers   = 1,
			.samples	   = num_samples,
			.tiling		   = tiling,
			.usage		   = usage,
			.sharingMode   = vk::SharingMode::eExclusive,
			.initialLayout = vk::ImageLayout::eUndefined,
		};

		VmaAllocationCreateInfo vma_alloc_info{};
		vma_alloc_info.usage		 = VMA_MEMORY_USAGE_UNKNOWN;
		vma_alloc_info.flags		 = 0;
		vma_alloc_info.requiredFlags = static_cast<VkMemoryPropertyFlags>(properties);

		native_handle = device->createImage(create_info);

		if (vmaAllocateMemoryForImage(
				allocator->getHandle(),
				*native_handle,
				&vma_alloc_info,
				&allocation,
				&allocation_info
			) != VK_SUCCESS)
		{
			throw ENGINE_EXCEPTION("Could not allocate image memory!");
		}

		if (vmaBindImageMemory(allocator->getHandle(), allocation, *native_handle) !=
			VK_SUCCESS)
		{
			throw ENGINE_EXCEPTION("Could not bind image memory!");
		}

		vmaSetAllocationName(allocator->getHandle(), allocation, "Image");
	}

	Image::~Image()
	{
		// Ensure image is released before memory is deallocated
		native_handle.clear();
		vmaFreeMemory(allocator->getHandle(), allocation);
	}

	void Image::transitionImageLayout(
		CommandBuffer&	with_command_buffer,
		vk::ImageLayout new_layout
	)
	{
		vk::ImageMemoryBarrier barrier{
			.oldLayout			 = current_layout,
			.newLayout			 = new_layout,
			.srcQueueFamilyIndex = vk::QueueFamilyIgnored,
			.dstQueueFamilyIndex = vk::QueueFamilyIgnored,
			.image				 = native_handle,
			.subresourceRange	 = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1},
		};

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
		else { throw ENGINE_EXCEPTION("Unsupported layout transition!"); }

		// Create the barrier that performs this action
		{
			with_command_buffer.pipelineBarrier(src_stage, dst_stage, {}, {}, {}, barrier);
		}

		current_layout = new_layout;
	}

	ViewedImage::ViewedImage(
		LogicalDevice*			with_device,
		MemoryAllocator*		with_allocator,
		ImageSize				with_size,
		vk::SampleCountFlagBits num_samples,
		vk::Format				format,
		vk::ImageUsageFlags		usage,
		vk::ImageAspectFlags	with_aspect_flags,
		vk::MemoryPropertyFlags properties,
		vk::ImageTiling			with_tiling
	)
		: Image(
			  with_device,
			  with_allocator,
			  with_size,
			  num_samples,
			  format,
			  usage,
			  properties,
			  with_tiling
		  )
	{
		vk::ImageViewCreateInfo create_info{
			.image	  = native_handle,
			.viewType = vk::ImageViewType::e2D,
			.format	  = image_format,
			.subresourceRange =
				{.aspectMask	 = with_aspect_flags,
				 .baseMipLevel	 = 0,
				 .levelCount	 = 1,
				 .baseArrayLayer = 0,
				 .layerCount	 = 1}
		};

		view_handle = device->createImageView(create_info);
	}

} // namespace Engine::Rendering::Vulkan
