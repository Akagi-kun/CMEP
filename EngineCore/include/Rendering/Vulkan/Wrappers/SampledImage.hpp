#pragma once

#include "Rendering/Vulkan/Wrappers/Image.hpp"

#include "vulkan/vulkan_core.h"

namespace Engine::Rendering::Vulkan
{
	class SampledImage : public Image
	{
	private:
		VkFilter use_filter;
		VkSamplerAddressMode use_address_mode;

	public:
		VkSampler texture_sampler;

		SampledImage(
			InstanceOwned::value_t with_instance,
			ImageSize with_size,
			VkSampleCountFlagBits num_samples,
			VkFormat format,
			VkImageUsageFlags usage,
			VkFilter with_filter,
			VkSamplerAddressMode with_address_mode,
			VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VkImageTiling with_tiling		 = VK_IMAGE_TILING_OPTIMAL
		);

		~SampledImage();
	};
} // namespace Engine::Rendering::Vulkan
