#pragma once

#include "Rendering/Vulkan/VImage.hpp"

namespace Engine::Rendering::Vulkan
{
	class VSampledImage : public VImage
	{
	private:
		VkFilter use_filter;
		VkSamplerAddressMode use_address_mode;

	public:
		VkSampler texture_sampler;

		VSampledImage(
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

		~VSampledImage();
	};
} // namespace Engine::Rendering::Vulkan
