#include "objects/SampledImage.hpp"

namespace Engine::Rendering::Vulkan
{
	template <typename base_t>
	[[nodiscard]] const vk::SamplerCreateInfo& SampledImage<base_t>::GetSamplerCreateInfo(
		vk::Filter use_filter,
		vk::SamplerAddressMode use_address_mode,
		float max_anisotropy
	)
	{
		static const vk::SamplerCreateInfo create_info = {
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
