#pragma once

#include "Assets/Texture.hpp"

#include "InternalEngineObject.hpp"

namespace Engine::Factories
{
	class TextureFactory : public InternalEngineObject
	{
	public:
		using InternalEngineObject::InternalEngineObject;

		std::shared_ptr<Rendering::Texture> InitFile(
			const std::filesystem::path& path,
			Rendering::Texture_InitFiletype filetype,
			vk::Filter filtering						= vk::Filter::eLinear,
			vk::SamplerAddressMode sampler_address_mode = vk::SamplerAddressMode::eRepeat
		);

	private:
		int InitRaw(
			std::unique_ptr<Rendering::TextureData>& texture_data,
			std::vector<unsigned char> raw_data,
			int color_format,
			vk::Filter filtering,
			vk::SamplerAddressMode sampler_address_mode,
			Rendering::ImageSize size
		);
	};
} // namespace Engine::Factories
