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
				.magFilter				 = use_filter,
				.minFilter				 = use_filter,
				.mipmapMode				 = vk::SamplerMipmapMode::eLinear,
				.addressModeU			 = use_address_mode,
				.addressModeV			 = use_address_mode,
				.addressModeW			 = use_address_mode,
				.mipLodBias				 = {},
				.anisotropyEnable		 = vk::True,
				.maxAnisotropy			 = max_anisotropy,
				.compareEnable			 = vk::False,
				.compareOp				 = {},
				.minLod					 = 0.f,
				.maxLod					 = 0.f,
				.borderColor			 = vk::BorderColor::eIntOpaqueBlack,
				.unnormalizedCoordinates = vk::False
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
	{}
} // namespace Engine::Rendering::Vulkan
