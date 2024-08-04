#include "Rendering/Vulkan/Wrappers/VSampledImage.hpp"

#include "Rendering/Vulkan/VDeviceManager.hpp"

#include <stdexcept>

namespace Engine::Rendering::Vulkan
{
	VSampledImage::VSampledImage(
		VDeviceManager* const with_device_manager,
		VImageSize with_size,
		VkSampleCountFlagBits num_samples,
		VkFormat format,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkFilter with_filter,
		VkSamplerAddressMode with_address_mode,
		VkImageTiling with_tiling
	)
		: VImage(with_device_manager, with_size, num_samples, format, usage, properties, with_tiling),
		  use_filter(with_filter), use_address_mode(with_address_mode)
	{
		VkSamplerCreateInfo sampler_info{};
		sampler_info.sType	   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_info.magFilter = this->use_filter;
		sampler_info.minFilter = this->use_filter; // VK_FILTER_LINEAR;

		sampler_info.addressModeU = this->use_address_mode; // VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_info.addressModeV = this->use_address_mode;
		sampler_info.addressModeW = this->use_address_mode;

		VkPhysicalDeviceProperties device_properties{};
		vkGetPhysicalDeviceProperties(this->device_manager->GetPhysicalDevice(), &device_properties);

		sampler_info.anisotropyEnable = VK_TRUE;
		sampler_info.maxAnisotropy	  = device_properties.limits.maxSamplerAnisotropy;

		sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

		sampler_info.unnormalizedCoordinates = VK_FALSE;

		sampler_info.compareEnable = VK_FALSE;
		sampler_info.compareOp	   = VK_COMPARE_OP_ALWAYS;

		sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_info.mipLodBias = 0.0f;
		sampler_info.minLod		= 0.0f;
		sampler_info.maxLod		= 0.0f;

		if (vkCreateSampler(
				this->device_manager->GetLogicalDevice(),
				&sampler_info,
				nullptr,
				&(this->texture_sampler)
			) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture sampler!");
		}
	}

	VSampledImage::~VSampledImage()
	{
		vkDestroySampler(this->device_manager->GetLogicalDevice(), this->texture_sampler, nullptr);
	}
} // namespace Engine::Rendering::Vulkan
