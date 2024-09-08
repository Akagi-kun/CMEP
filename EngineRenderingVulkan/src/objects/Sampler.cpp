#include "objects/Sampler.hpp"

#include "backend/LogicalDevice.hpp"

namespace Engine::Rendering::Vulkan
{
	namespace
	{
		[[nodiscard]] vk::SamplerCreateInfo getSamplerCreateInfo(
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
	} // namespace

	Sampler::Sampler(
		LogicalDevice*		   with_device,
		vk::Filter			   with_filter,
		vk::SamplerAddressMode with_address_mode,
		float				   with_max_anisotropy
	)
		: vk::raii::Sampler(with_device->createSampler(
			  getSamplerCreateInfo(with_filter, with_address_mode, with_max_anisotropy)
		  )),
		  filter(with_filter), address_mode(with_address_mode)
	{
	}
} // namespace Engine::Rendering::Vulkan
