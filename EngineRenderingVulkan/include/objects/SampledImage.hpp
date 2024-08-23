#pragma once

#include "fwd.hpp"

#include "Rendering/Transform.hpp"

#include "ImportVulkan.hpp"
#include "backend/LogicalDevice.hpp"
#include "common/InstanceOwned.hpp"
#include "objects/Image.hpp"

namespace Engine::Rendering::Vulkan
{
	template <typename base_t> class SampledImage : public base_t
	{
	public:
		using self_t = SampledImage<base_t>;

		vk::raii::Sampler texture_sampler = nullptr;

		template <
			typename conditional_t											= int,
			std::enable_if_t<std::is_same_v<base_t, Image>, conditional_t>* = nullptr>
		SampledImage<base_t>(
			InstanceOwned::value_t with_instance,
			ImageSize with_size,
			vk::SampleCountFlagBits num_samples,
			vk::Format format,
			vk::ImageUsageFlags usage,
			vk::Filter with_filter,
			vk::SamplerAddressMode with_address_mode,
			vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eDeviceLocal,
			vk::ImageTiling with_tiling		   = vk::ImageTiling::eOptimal
		)
			: Image(with_instance, with_size, num_samples, format, usage, properties, with_tiling),
			  use_filter(with_filter), use_address_mode(with_address_mode)
		{

			LogicalDevice* logical_device = this->instance->GetLogicalDevice();

			vk::PhysicalDeviceProperties device_properties = this->instance->GetPhysicalDevice()->getProperties();

			texture_sampler = logical_device->createSampler(
				GetSamplerCreateInfo(use_filter, use_address_mode, device_properties.limits.maxSamplerAnisotropy)
			);
		}

		template <
			typename conditional_t												  = int,
			std::enable_if_t<std::is_same_v<base_t, ViewedImage>, conditional_t>* = nullptr>
		SampledImage<base_t>(
			InstanceOwned::value_t with_instance,
			ImageSize with_size,
			vk::SampleCountFlagBits num_samples,
			vk::Format format,
			vk::ImageUsageFlags usage,
			vk::Filter with_filter,
			vk::SamplerAddressMode with_address_mode,
			vk::ImageAspectFlags with_aspect,
			vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eDeviceLocal,
			vk::ImageTiling with_tiling		   = vk::ImageTiling::eOptimal
		)
			: ViewedImage(with_instance, with_size, num_samples, format, usage, with_aspect, properties, with_tiling),
			  use_filter(with_filter), use_address_mode(with_address_mode)
		{
			LogicalDevice* logical_device = this->instance->GetLogicalDevice();

			vk::PhysicalDeviceProperties device_properties = this->instance->GetPhysicalDevice()->getProperties();

			texture_sampler = logical_device->createSampler(
				GetSamplerCreateInfo(use_filter, use_address_mode, device_properties.limits.maxSamplerAnisotropy)
			);
		}

		~SampledImage() = default;

	private:
		vk::Filter use_filter;
		vk::SamplerAddressMode use_address_mode;

		[[nodiscard]] static vk::SamplerCreateInfo GetSamplerCreateInfo(
			vk::Filter use_filter,
			vk::SamplerAddressMode use_address_mode,
			float max_anisotropy
		);
	};
} // namespace Engine::Rendering::Vulkan
