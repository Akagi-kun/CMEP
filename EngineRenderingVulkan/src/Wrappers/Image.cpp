#include "Wrappers/Image.hpp"

#include "ImportVulkan.hpp"
#include "Wrappers/CommandBuffer.hpp"
#include "Wrappers/CommandPool.hpp"
#include "Wrappers/Instance.hpp"
#include "Wrappers/framework.hpp"
#include "vulkan/vulkan_core.h"

#include <stdexcept>

namespace Engine::Rendering::Vulkan
{
	Image::Image(
		InstanceOwned::value_t with_instance,
		ImageSize with_size,
		vk::SampleCountFlagBits num_samples,
		VkFormat format,
		VkImageUsageFlags usage,
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
		image_info.format		 = format;
		image_info.tiling		 = tiling;
		image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_info.usage		 = usage;
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
				&(native_handle),
				&(this->allocation),
				&(this->allocation_info)
			) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create VulkanImage!");
		}

		vmaSetAllocationName(*this->allocator, this->allocation, "VImage");
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

	void Image::TransitionImageLayout(VkImageLayout new_layout)
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

		barrier.oldLayout = this->current_layout;
		barrier.newLayout = new_layout;

		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.image = native_handle;

		barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel	= 0;
		barrier.subresourceRange.levelCount		= 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount		= 1;

		VkPipelineStageFlags source_stage;
		VkPipelineStageFlags destination_stage;

		if (this->current_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			source_stage	  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (this->current_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
				 new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			source_stage	  = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
		{
			throw std::invalid_argument("Unsupported layout transition!");
		}

		auto* command_buffer = instance->GetCommandPool()->AllocateCommandBuffer();

		// VCommandBuffer(this->device_manager, this->device_manager->GetCommandPool())
		command_buffer->RecordCmds([&](CommandBuffer* with_buffer) {
			vkCmdPipelineBarrier(*with_buffer, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		});

		delete command_buffer;

		this->current_layout = new_layout;
	}

	void Image::AddImageView(VkImageAspectFlags with_aspect_flags)
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		VkImageViewCreateInfo view_info{};
		view_info.sType							  = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image							  = native_handle;
		view_info.viewType						  = VK_IMAGE_VIEW_TYPE_2D;
		view_info.format						  = this->image_format;
		view_info.subresourceRange.aspectMask	  = with_aspect_flags;
		view_info.subresourceRange.baseMipLevel	  = 0;
		view_info.subresourceRange.levelCount	  = 1;
		view_info.subresourceRange.baseArrayLayer = 0;
		view_info.subresourceRange.layerCount	  = 1;

		if (vkCreateImageView(logical_device->GetHandle(), &view_info, nullptr, &native_view_handle) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture image view!");
		}
	}
} // namespace Engine::Rendering::Vulkan
