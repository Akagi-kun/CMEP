#include "Wrappers/Image.hpp"

#include "ImportVulkan.hpp"
#include "Wrappers/CommandBuffer.hpp"
#include "Wrappers/CommandPool.hpp"
#include "Wrappers/Instance.hpp"
#include "Wrappers/framework.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_handles.hpp"
#include "vulkan/vulkan_structs.hpp"

#include <stdexcept>

namespace Engine::Rendering::Vulkan
{
	Image::Image(
		InstanceOwned::value_t with_instance,
		ImageSize with_size,
		vk::SampleCountFlagBits num_samples,
		vk::Format format,
		vk::ImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkImageTiling tiling
	)
		: InstanceOwned(with_instance), HoldsVMA(with_instance->GetGraphicMemoryAllocator()), image_format(format),
		  size(with_size)
	{
		VkImageCreateInfo image_info{};
		image_info.sType		 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.imageType	 = VK_IMAGE_TYPE_2D;
		image_info.extent		 = VkExtent3D{size.x, size.y, 1};
		image_info.mipLevels	 = 1;
		image_info.arrayLayers	 = 1;
		image_info.format		 = static_cast<VkFormat>(format);
		image_info.tiling		 = tiling;
		image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_info.usage		 = static_cast<VkImageUsageFlags>(usage);
		image_info.sharingMode	 = VK_SHARING_MODE_EXCLUSIVE;
		image_info.samples		 = static_cast<VkSampleCountFlagBits>(num_samples);
		image_info.flags		 = 0; // Optional

		VmaAllocationCreateInfo vma_alloc_info{};
		vma_alloc_info.usage		 = VMA_MEMORY_USAGE_AUTO;
		vma_alloc_info.flags		 = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		// //VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
		// | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		vma_alloc_info.requiredFlags = properties;

		if (vmaCreateImage(
				*this->allocator,
				&image_info,
				&vma_alloc_info,
				reinterpret_cast<VkImage*>(&(native_handle)),
				&(this->allocation),
				&(this->allocation_info)
			) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create VulkanImage!");
		}

		vmaSetAllocationName(*this->allocator, this->allocation, "Image");
	}

	Image::~Image()
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		logical_device->WaitDeviceIdle();

		if (native_view_handle != nullptr)
		{
			vkDestroyImageView(logical_device->GetHandle(), native_view_handle, nullptr);
		}

		vkDestroyImage(logical_device->GetHandle(), native_handle, nullptr);

		vmaFreeMemory(*this->allocator, this->allocation);
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
			native_handle,
			{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1},
			{}
		);

		/*barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

		barrier.oldLayout = this->current_layout;
		barrier.newLayout = new_layout;

		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.image = native_handle;

		barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel	= 0;
		barrier.subresourceRange.levelCount		= 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount		= 1; */

		vk::PipelineStageFlags source_stage;
		vk::PipelineStageFlags destination_stage;

		if (current_layout == vk::ImageLayout::eUndefined /* VK_IMAGE_LAYOUT_UNDEFINED */ &&
			new_layout == vk::ImageLayout::eTransferDstOptimal /* VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL */)
		{
			barrier.srcAccessMask = {};
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

			// barrier.srcAccessMask = 0;
			// barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			source_stage	  = vk::PipelineStageFlagBits::eTopOfPipe;
			destination_stage = vk::PipelineStageFlagBits::eTransfer;

			// source_stage	  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			// destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (current_layout == vk::ImageLayout::eTransferDstOptimal /* VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL */ &&
				 new_layout == vk::ImageLayout::eShaderReadOnlyOptimal /* VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL */)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			// barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			// barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			source_stage	  = vk::PipelineStageFlagBits::eTransfer;
			destination_stage = vk::PipelineStageFlagBits::eFragmentShader;

			// source_stage	  = VK_PIPELINE_STAGE_TRANSFER_BIT;
			// destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
		{
			throw std::invalid_argument("Unsupported layout transition!");
		}

		auto* command_buffer = instance->GetCommandPool()->AllocateCommandBuffer();

		// VCommandBuffer(this->device_manager, this->device_manager->GetCommandPool())
		command_buffer->RecordCmds([&](CommandBuffer* with_buffer) {
			with_buffer->GetHandle().pipelineBarrier(source_stage, destination_stage, {}, 0, {}, 0, {}, 1, &barrier);

			/* vkCmdPipelineBarrier(
				with_buffer->GetHandle(),
				source_stage,
				destination_stage,
				0,
				0,
				nullptr,
				0,
				nullptr,
				1,
				&barrier
			); */
		});

		delete command_buffer;

		this->current_layout = new_layout;
	}

	void Image::AddImageView(vk::ImageAspectFlags with_aspect_flags)
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		vk::ImageViewCreateInfo view_create_info(
			{},
			native_handle,
			vk::ImageViewType::e2D,
			image_format,
			{},
			{with_aspect_flags, 0, 1, 0, 1}
		);

		/* view_info.sType							  = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image							  = native_handle;
		view_info.viewType						  = VK_IMAGE_VIEW_TYPE_2D;
		view_info.format						  = image_format;
		view_info.subresourceRange.aspectMask	  = with_aspect_flags;
		view_info.subresourceRange.baseMipLevel	  = 0;
		view_info.subresourceRange.levelCount	  = 1;
		view_info.subresourceRange.baseArrayLayer = 0;
		view_info.subresourceRange.layerCount	  = 1; */

		native_view_handle = logical_device->GetHandle().createImageView(view_create_info);

		/* if (vkCreateImageView(logical_device->GetHandle(), &view_create_info, nullptr, &native_view_handle) !=
			VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture image view!");
		} */
	}
} // namespace Engine::Rendering::Vulkan
