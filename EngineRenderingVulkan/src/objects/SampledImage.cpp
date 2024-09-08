#include "objects/SampledImage.hpp"

#include "Rendering/Transform.hpp"

#include "backend/Instance.hpp"
#include "backend/LogicalDevice.hpp"

namespace Engine::Rendering::Vulkan
{
	template <typename base_t>
	SampledImage<base_t>::SampledImage(
		InstanceOwned::value_t	with_instance,
		ImageSize				with_size,
		vk::SampleCountFlagBits num_samples,
		vk::Format				format,
		vk::ImageUsageFlags		usage,
		vk::Filter				with_filter,
		vk::SamplerAddressMode	with_address_mode,
		vk::MemoryPropertyFlags properties,
		vk::ImageTiling			with_tiling
	)
		requires(std::is_same_v<base_t, Image>)
		: Image(
			  with_instance,
			  with_size,
			  num_samples,
			  format,
			  usage,
			  properties,
			  with_tiling
		  ),
		  use_filter(with_filter), use_address_mode(with_address_mode)
	{

		LogicalDevice* logical_device = this->instance->getLogicalDevice();

		vk::PhysicalDeviceProperties device_properties =
			this->instance->getPhysicalDevice()->getProperties();

		texture_sampler = logical_device->createSampler(getSamplerCreateInfo(
			use_filter,
			use_address_mode,
			device_properties.limits.maxSamplerAnisotropy
		));
	}

	template <typename base_t>
	SampledImage<base_t>::SampledImage(
		InstanceOwned::value_t	with_instance,
		ImageSize				with_size,
		vk::SampleCountFlagBits num_samples,
		vk::Format				format,
		vk::ImageUsageFlags		usage,
		vk::Filter				with_filter,
		vk::SamplerAddressMode	with_address_mode,
		vk::ImageAspectFlags	with_aspect,
		vk::MemoryPropertyFlags properties,
		vk::ImageTiling			with_tiling
	)
		requires(std::is_same_v<base_t, ViewedImage>)
		: ViewedImage(
			  with_instance,
			  with_size,
			  num_samples,
			  format,
			  usage,
			  with_aspect,
			  properties,
			  with_tiling
		  ),
		  use_filter(with_filter), use_address_mode(with_address_mode)
	{
		LogicalDevice* logical_device = this->instance->getLogicalDevice();

		vk::PhysicalDeviceProperties device_properties =
			this->instance->getPhysicalDevice()->getProperties();

		texture_sampler = logical_device->createSampler(getSamplerCreateInfo(
			use_filter,
			use_address_mode,
			device_properties.limits.maxSamplerAnisotropy
		));
	}

	template <typename base_t>
	[[nodiscard]] vk::SamplerCreateInfo SampledImage<base_t>::getSamplerCreateInfo(
		vk::Filter			   use_filter,
		vk::SamplerAddressMode use_address_mode,
		float				   max_anisotropy
	)
	{
		const vk::SamplerCreateInfo create_info = {
			{},
			use_filter,
			use_filter,
			vk::SamplerMipmapMode::eLinear,
			use_address_mode,
			use_address_mode,
			use_address_mode,
			{},
			vk::True,
			max_anisotropy,
			vk::False,
			{},
			0.f,
			0.f,
			vk::BorderColor::eIntOpaqueBlack,
			vk::False,
			{},
		};

		return create_info;
	}

	// Instantiate templates
	template class SampledImage<Image>;
	template class SampledImage<ViewedImage>;
} // namespace Engine::Rendering::Vulkan
