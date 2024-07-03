#pragma once

#include "Rendering/Vulkan/VulkanImage.hpp"

namespace Engine::Rendering
{
	class VulkanTextureImage : public VulkanImage
	{
	private:
		VkFilter use_filter;
		VkSamplerAddressMode use_address_mode;

	public:
		VkSampler texture_sampler;

		VulkanTextureImage(
			VulkanDeviceManager* with_device_manager,
			VmaAllocator with_allocator,
			VulkanImageSize size,
			VkSampleCountFlagBits num_samples,
			VkFormat format,
			VkImageTiling tiling,
			VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkFilter with_filter,
			VkSamplerAddressMode with_address_mode
		);

		~VulkanTextureImage();
	};
} // namespace Engine::Rendering
