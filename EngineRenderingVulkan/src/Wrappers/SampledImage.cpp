#include "Wrappers/SampledImage.hpp"

#include "Wrappers/Instance.hpp"
#include "Wrappers/LogicalDevice.hpp"
#include "Wrappers/PhysicalDevice.hpp"

#include <stdexcept>

namespace Engine::Rendering::Vulkan
{
	SampledImage::SampledImage(
		InstanceOwned::value_t with_instance,
		ImageSize with_size,
		vk::SampleCountFlagBits num_samples,
		vk::Format format,
		vk::ImageUsageFlags usage,
		VkFilter with_filter,
		VkSamplerAddressMode with_address_mode,
		VkMemoryPropertyFlags properties,
		VkImageTiling with_tiling
	)
		: Image(with_instance, with_size, num_samples, format, usage, properties, with_tiling), use_filter(with_filter),
		  use_address_mode(with_address_mode)
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		VkSamplerCreateInfo sampler_info{};
		sampler_info.sType	   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_info.magFilter = use_filter;
		sampler_info.minFilter = use_filter;

		sampler_info.addressModeU = use_address_mode;
		sampler_info.addressModeV = use_address_mode;
		sampler_info.addressModeW = use_address_mode;

		VkPhysicalDeviceProperties device_properties{};
		vkGetPhysicalDeviceProperties(
			static_cast<PhysicalDevice::value_t>(instance->GetPhysicalDevice()),
			&device_properties
		);

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

		if (vkCreateSampler(logical_device->GetHandle(), &sampler_info, nullptr, &(texture_sampler)) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture sampler!");
		}
	}

	SampledImage::~SampledImage()
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		vkDestroySampler(logical_device->GetHandle(), texture_sampler, nullptr);
	}
} // namespace Engine::Rendering::Vulkan
