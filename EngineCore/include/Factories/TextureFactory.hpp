#pragma once

#include "Assets/Texture.hpp"
#include "Rendering/Transform.hpp"

#include "InternalEngineObject.hpp"
#include "vulkan/vulkan_enums.hpp"

#include <filesystem>
#include <memory>
#include <vector>

namespace Engine::Factories
{
	class TextureFactory : public InternalEngineObject
	{
	public:
		using InternalEngineObject::InternalEngineObject;

		[[nodiscard]] std::shared_ptr<Rendering::Texture> createTexture(
			const std::filesystem::path& path,
			vk::Filter					 filtering		= vk::Filter::eLinear,
			vk::SamplerAddressMode sampler_address_mode = vk::SamplerAddressMode::eRepeat
		);

	private:
		int createTextureInternal(
			std::unique_ptr<Rendering::TextureData>& texture_data,
			std::vector<unsigned char>				 raw_data,
			int										 color_format,
			vk::Filter								 filtering,
			vk::SamplerAddressMode					 sampler_address_mode,
			Rendering::ImageSize					 size
		);
	};
} // namespace Engine::Factories
