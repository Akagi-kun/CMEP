#pragma once

#include "VulkanRenderingEngine.hpp"
#include "InternalEngineObject.hpp"

namespace Engine::Rendering::Factories
{
/* 	struct VulkanImage
	{
		VkImage image;
		VmaAllocation allocation;
		VmaAllocationInfo allocationInfo;
		VkFormat imageFormat;
		VkImageView imageView;
	};

	struct VulkanTextureImage
	{
		VulkanImage* image;
		VkSampler textureSampler;
		VkFilter useFilter;
		VkSamplerAddressMode useAddressMode;
	};
 */
	class VulkanImageFactory : public InternalEngineObject
	{
	protected:
		VmaAllocator vmaAllocator;
		VulkanRenderingEngine* vulkanRenderingEngine;

	public:
		VulkanImageFactory(VmaAllocator vmaAllocator, VulkanRenderingEngine* vre) : vmaAllocator(vmaAllocator), vulkanRenderingEngine(vre) {};

		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

		VulkanImage* createImage(uint32_t width, uint32_t height, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
		VulkanTextureImage* createTextureImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkFilter useFilter, VkSamplerAddressMode addressMode);
		void appendImageViewToTextureImage(VulkanTextureImage* teximage);
		//void appendVulkanSamplerToVulkanTextureImage(VulkanTextureImage* teximage);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	};
}