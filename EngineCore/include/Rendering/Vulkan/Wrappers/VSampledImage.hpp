#pragma once

#include "Rendering/Vulkan/Wrappers/VImage.hpp"

#include "vulkan/vulkan_core.h"

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
			VDeviceManager* with_device_manager,
			VImageSize with_size,
			VkSampleCountFlagBits num_samples,
			VkFormat format,
			VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkFilter with_filter,
			VkSamplerAddressMode with_address_mode,
			VkImageTiling with_tiling = VK_IMAGE_TILING_OPTIMAL
		);

		~VSampledImage();
	};
} // namespace Engine::Rendering::Vulkan
