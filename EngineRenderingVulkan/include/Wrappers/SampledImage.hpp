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
			vk::Filter with_filter,
			vk::SamplerAddressMode with_address_mode,
			vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eDeviceLocal,
			vk::ImageTiling with_tiling		   = vk::ImageTiling::eOptimal
		);

		~SampledImage();

	private:
		const vk::Filter use_filter;
		const vk::SamplerAddressMode use_address_mode;
	};
} // namespace Engine::Rendering::Vulkan
