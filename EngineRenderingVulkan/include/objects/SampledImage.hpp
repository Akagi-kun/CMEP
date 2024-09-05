#pragma once

#include "fwd.hpp"

#include "Rendering/Transform.hpp"

#include "common/InstanceOwned.hpp"
#include "objects/Image.hpp"
#include "vulkan/vulkan_raii.hpp"

namespace Engine::Rendering::Vulkan
{
	template <typename base_t> class SampledImage : public base_t
	{
	public:
		using self_t = SampledImage<base_t>;

		vk::raii::Sampler texture_sampler = nullptr;

		SampledImage(
			InstanceOwned::value_t	with_instance,
			ImageSize				with_size,
			vk::SampleCountFlagBits num_samples,
			vk::Format				format,
			vk::ImageUsageFlags		usage,
			vk::Filter				with_filter,
			vk::SamplerAddressMode	with_address_mode,
			vk::MemoryPropertyFlags properties	= vk::MemoryPropertyFlagBits::eDeviceLocal,
			vk::ImageTiling			with_tiling = vk::ImageTiling::eOptimal
		)
			requires(std::is_same_v<base_t, Image>);

		SampledImage(
			InstanceOwned::value_t	with_instance,
			ImageSize				with_size,
			vk::SampleCountFlagBits num_samples,
			vk::Format				format,
			vk::ImageUsageFlags		usage,
			vk::Filter				with_filter,
			vk::SamplerAddressMode	with_address_mode,
			vk::ImageAspectFlags	with_aspect,
			vk::MemoryPropertyFlags properties	= vk::MemoryPropertyFlagBits::eDeviceLocal,
			vk::ImageTiling			with_tiling = vk::ImageTiling::eOptimal
		)
			requires(std::is_same_v<base_t, ViewedImage>);

		~SampledImage() = default;

	private:
		vk::Filter			   use_filter;
		vk::SamplerAddressMode use_address_mode;

		[[nodiscard]] static vk::SamplerCreateInfo GetSamplerCreateInfo(
			vk::Filter			   use_filter,
			vk::SamplerAddressMode use_address_mode,
			float				   max_anisotropy
		);
	};
} // namespace Engine::Rendering::Vulkan
