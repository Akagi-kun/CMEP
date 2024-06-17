#pragma once

#include "InternalEngineObject.hpp"
#include "VulkanRenderingEngine.hpp"

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
		VmaAllocator vma_allocator;
		VulkanRenderingEngine* vulkan_rendering_engine;

	public:
		VulkanImageFactory(VmaAllocator with_allocator, VulkanRenderingEngine* with_vre)
			: vma_allocator(with_allocator), vulkan_rendering_engine(with_vre)
		{
		}

		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

		VulkanImage* CreateImage(
			uint32_t width,
			uint32_t height,
			VkSampleCountFlagBits numSamples,
			VkFormat format,
			VkImageTiling tiling,
			VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties
		);

		VulkanTextureImage* CreateTextureImage(
			uint32_t width,
			uint32_t height,
			VkFormat format,
			VkImageTiling tiling,
			VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkFilter useFilter,
			VkSamplerAddressMode addressMode
		);

		void AppendImageViewToTextureImage(VulkanTextureImage* teximage);

		// void appendVulkanSamplerToVulkanTextureImage(VulkanTextureImage* teximage);

		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	};
} // namespace Engine::Rendering::Factories
