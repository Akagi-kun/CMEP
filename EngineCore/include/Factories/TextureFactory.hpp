#pragma once

#include "Rendering/Texture.hpp"

#include "Rendering/Vulkan/VulkanStructDefs.hpp"

#include "InternalEngineObject.hpp"

namespace Engine::Factories
{
	class TextureFactory : public InternalEngineObject
	{
	private:
		int InitRaw(
			std::unique_ptr<Rendering::TextureData>& texture_data,
			Rendering::VulkanBuffer* staging_buffer,
			std::vector<unsigned char> raw_data,
			int color_format,
			unsigned int xsize,
			unsigned int ysize
		);

	public:
		TextureFactory();

		std::shared_ptr<Rendering::Texture> InitFile(
			std::string path,
			Rendering::VulkanBuffer* staging_buffer,
			Rendering::Texture_InitFiletype filetype,
			unsigned int sizex,
			unsigned int sizey
		);
	};
} // namespace Engine::Factories