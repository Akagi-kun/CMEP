#pragma once

#include "Assets/Texture.hpp"
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
			VkFilter filtering,
			VkSamplerAddressMode sampler_address_mode,
			unsigned int xsize,
			unsigned int ysize
		);

	public:
		TextureFactory() = default;

		std::shared_ptr<Rendering::Texture> InitFile(
			const std::string& path,
			Rendering::VulkanBuffer* staging_buffer,
			Rendering::Texture_InitFiletype filetype,
			VkFilter filtering						  = VK_FILTER_LINEAR,
			VkSamplerAddressMode sampler_address_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			unsigned int sizex						  = 0,
			unsigned int sizey						  = 0
		);
	};
} // namespace Engine::Factories
