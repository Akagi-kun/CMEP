#include "Wrappers/SampledImage.hpp"

#include "Wrappers/Instance.hpp"
#include "Wrappers/LogicalDevice.hpp"
#include "Wrappers/PhysicalDevice.hpp"

namespace Engine::Rendering::Vulkan
{
	SampledImage::SampledImage(
		InstanceOwned::value_t with_instance,
		ImageSize with_size,
		vk::SampleCountFlagBits num_samples,
		vk::Format format,
		vk::ImageUsageFlags usage,
		vk::Filter with_filter,
		vk::SamplerAddressMode with_address_mode,
		vk::MemoryPropertyFlags properties,
		vk::ImageTiling with_tiling
	)
		: Image(with_instance, with_size, num_samples, format, usage, properties, with_tiling), use_filter(with_filter),
		  use_address_mode(with_address_mode)
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		vk::PhysicalDeviceProperties device_properties = instance->GetPhysicalDevice().GetHandle().getProperties();

		texture_sampler = logical_device->GetHandle().createSampler({
			{},
			use_filter,
			use_filter,
			vk::SamplerMipmapMode::eLinear,
			use_address_mode,
			use_address_mode,
			use_address_mode,
			{},
			vk::True,
			device_properties.limits.maxSamplerAnisotropy,
			vk::False,
			{}, // ?
			0.f,
			0.f,
			vk::BorderColor::eIntOpaqueBlack,
			vk::False,
			{},
		});
	}

	SampledImage::~SampledImage()
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		logical_device->GetHandle().destroySampler(texture_sampler);
	}
} // namespace Engine::Rendering::Vulkan
