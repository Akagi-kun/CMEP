#include "Rendering/Vulkan/VulkanImageFactory.hpp"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_VULKAN_IMAGE_FACTORY
#include "Logging/LoggingPrefix.hpp"

namespace Engine::Rendering::Factories
{
	VkImageView VulkanImageFactory::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		if (vkCreateImageView(this->vulkanRenderingEngine->GetLogicalDevice(), &viewInfo, nullptr, &imageView) !=
			VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture image view!");
		}

		return imageView;
	}

	VulkanImage* VulkanImageFactory::createImage(
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

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = static_cast<uint32_t>(width);
		imageInfo.extent.height = static_cast<uint32_t>(height);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = numSamples;
		imageInfo.flags = 0; // Optional

		VmaAllocationCreateInfo vmaAllocInfo{};
		vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		vmaAllocInfo.flags =
			VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT; // vmaAllocFlags;
														// //VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
														// | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		vmaAllocInfo.requiredFlags = properties;

		// if (vkCreateImage(this->vkLogicalDevice, &imageInfo, nullptr, &(new_image->image)) != VK_SUCCESS)
		if (vmaCreateImage(
				this->vmaAllocator,
				&imageInfo,
				&vmaAllocInfo,
				&(new_image->image),
				&(new_image->allocation),
				&(new_image->allocationInfo)
			) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Exception, LOGPFX_CURRENT "Failed to create image");
			throw std::runtime_error("failed to create image!");
		}

		vmaSetAllocationName(this->vmaAllocator, new_image->allocation, "VulkanImage");

		return new_image;
	}

	VulkanTextureImage* VulkanImageFactory::createTextureImage(
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

		new_texture_image->image = this->createImage(
			width, height, VK_SAMPLE_COUNT_1_BIT, format, tiling, usage, properties
		);
		new_texture_image->useAddressMode = addressMode;
		new_texture_image->useFilter = useFilter;

		return new_texture_image;
	}

	void VulkanImageFactory::appendImageViewToTextureImage(VulkanTextureImage* teximage)
	{
		teximage->image->imageView = this->createImageView(
			teximage->image->image, teximage->image->imageFormat, VK_IMAGE_ASPECT_COLOR_BIT
		);
	}

	// void appendVulkanSamplerToVulkanTextureImage(VulkanTextureImage* teximage);
	void VulkanImageFactory::transitionImageLayout(
		VkImage image,
		VkFormat format,
		VkImageLayout oldLayout,
		VkImageLayout newLayout
	)
	{
		VkCommandBuffer commandBuffer = this->vulkanRenderingEngine->beginVulkanSingleTimeCommandsCommandBuffer();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
				 newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
		{
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Unsupported layout transition requested");
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		this->vulkanRenderingEngine->endVulkanSingleTimeCommandsCommandBuffer(commandBuffer);
	}
} // namespace Engine::Rendering::Factories
