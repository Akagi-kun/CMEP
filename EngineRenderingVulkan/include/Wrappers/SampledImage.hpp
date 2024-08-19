#pragma once

#include "ImportVulkan.hpp"
#include "Wrappers/Image.hpp"

namespace Engine::Rendering::Vulkan
{
	class SampledImage : public Image
	{
	public:
		VkSampler texture_sampler;

		SampledImage(
			InstanceOwned::value_t with_instance,
			ImageSize with_size,
			vk::SampleCountFlagBits num_samples,
			vk::Format format,
			vk::ImageUsageFlags usage,
			VkFilter with_filter,
			VkSamplerAddressMode with_address_mode,
			VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VkImageTiling with_tiling		 = VK_IMAGE_TILING_OPTIMAL
		);

		~SampledImage();

	private:
		VkFilter use_filter;
		VkSamplerAddressMode use_address_mode;
	};
} // namespace Engine::Rendering::Vulkan
