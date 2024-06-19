#include "Rendering/Vulkan/VulkanImageFactory.hpp"

#include <stdexcept>

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_VULKAN_IMAGE_FACTORY
#include "Logging/LoggingPrefix.hpp"

namespace Engine::Rendering::Factories
{
	VkImageView VulkanImageFactory::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
	{
		VkImageViewCreateInfo view_info{};
		view_info.sType							  = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image							  = image;
		view_info.viewType						  = VK_IMAGE_VIEW_TYPE_2D;
		view_info.format						  = format;
		view_info.subresourceRange.aspectMask	  = aspectFlags;
		view_info.subresourceRange.baseMipLevel	  = 0;
		view_info.subresourceRange.levelCount	  = 1;
		view_info.subresourceRange.baseArrayLayer = 0;
		view_info.subresourceRange.layerCount	  = 1;

		VkImageView image_view;
		if (vkCreateImageView(this->vulkan_rendering_engine->GetLogicalDevice(), &view_info, nullptr, &image_view) !=
			VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture image view!");
		}

		return image_view;
	}

	VulkanImage* VulkanImageFactory::CreateImage(
		uint32_t width,
		uint32_t height,
		VkSampleCountFlagBits numSamples,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties
	)
	{
		VulkanImage* new_image = new VulkanImage();

		new_image->imageFormat = format;

		VkImageCreateInfo image_info{};
		image_info.sType		 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.imageType	 = VK_IMAGE_TYPE_2D;
		image_info.extent.width	 = static_cast<uint32_t>(width);
		image_info.extent.height = static_cast<uint32_t>(height);
		image_info.extent.depth	 = 1;
		image_info.mipLevels	 = 1;
		image_info.arrayLayers	 = 1;
		image_info.format		 = format;
		image_info.tiling		 = tiling;
		image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_info.usage		 = usage;
		image_info.sharingMode	 = VK_SHARING_MODE_EXCLUSIVE;
		image_info.samples		 = numSamples;
		image_info.flags		 = 0; // Optional

		VmaAllocationCreateInfo vma_alloc_info{};
		vma_alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
		vma_alloc_info.flags =
			VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT; // vmaAllocFlags;
														// //VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
														// | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		vma_alloc_info.requiredFlags = properties;

		// if (vkCreateImage(this->vkLogicalDevice, &imageInfo, nullptr, &(new_image->image)) != VK_SUCCESS)
		if (vmaCreateImage(
				this->vma_allocator,
				&image_info,
				&vma_alloc_info,
				&(new_image->image),
				&(new_image->allocation),
				&(new_image->allocationInfo)
			) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Exception, LOGPFX_CURRENT "Failed to create image");
			throw std::runtime_error("failed to create image!");
		}

		vmaSetAllocationName(this->vma_allocator, new_image->allocation, "VulkanImage");

		return new_image;
	}

	VulkanTextureImage* VulkanImageFactory::CreateTextureImage(
		uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkFilter useFilter,
		VkSamplerAddressMode addressMode
	)
	{
		VulkanTextureImage* new_texture_image = new VulkanTextureImage();

		new_texture_image
			->image = this->CreateImage(width, height, VK_SAMPLE_COUNT_1_BIT, format, tiling, usage, properties);
		new_texture_image->useAddressMode = addressMode;
		new_texture_image->useFilter	  = useFilter;

		return new_texture_image;
	}

	void VulkanImageFactory::AppendImageViewToTextureImage(VulkanTextureImage* teximage)
	{
		teximage->image->imageView = this->CreateImageView(
			teximage->image->image,
			teximage->image->imageFormat,
			VK_IMAGE_ASPECT_COLOR_BIT
		);
	}

	// void appendVulkanSamplerToVulkanTextureImage(VulkanTextureImage* teximage);
	void VulkanImageFactory::TransitionImageLayout(
		VkImage image,
		VkFormat format,
		VkImageLayout oldLayout,
		VkImageLayout newLayout
	)
	{
		VkCommandBuffer command_buffer = this->vulkan_rendering_engine->BeginVulkanSingleTimeCommandsCommandBuffer();

		VkImageMemoryBarrier barrier{};
		barrier.sType							= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout						= oldLayout;
		barrier.newLayout						= newLayout;
		barrier.srcQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
		barrier.image							= image;
		barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel	= 0;
		barrier.subresourceRange.levelCount		= 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount		= 1;

		VkPipelineStageFlags source_stage;
		VkPipelineStageFlags destination_stage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			source_stage	  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
				 newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			source_stage	  = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
		{
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Unsupported layout transition requested");
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(command_buffer, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		this->vulkan_rendering_engine->EndVulkanSingleTimeCommandsCommandBuffer(command_buffer);
	}
} // namespace Engine::Rendering::Factories
