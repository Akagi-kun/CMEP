#pragma once

#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"

#include "InternalEngineObject.hpp"

#include <memory>
#include <string>
#include <vector>

namespace Engine::Rendering
{
	typedef enum class Texture_InitFiletypeEnum : int
	{
		FILE_PNG = 3
	} Texture_InitFiletype;

	struct TextureData
	{
		std::vector<unsigned char> data;
		uint_fast32_t x = 0, y = 0;
		int color_fmt = 4;
		VkFilter filtering;
		VkSamplerAddressMode address_mode;
		VulkanTextureImage* texture_image = nullptr;
	};

	class Texture final : public InternalEngineObject
	{
	private:
		/*
			VulkanBuffer* staging_buffer = nullptr;
			bool managedStagingBuffer = false;
	 */
		std::unique_ptr<TextureData> data;

	public:
		using InternalEngineObject::InternalEngineObject;
		~Texture();

		void Init(std::unique_ptr<TextureData> init_data);

		// void UsePremadeStagingBuffer(VulkanBuffer* staging_buffer);
		/*
				int InitRaw(std::vector<unsigned char> raw_data, int color_format, unsigned int xsize, unsigned int
		   ysize); int InitFile(Texture_InitFiletype filetype, std::string path, unsigned int sizex = 0, unsigned int
		   sizey = 0);
		 */
		void GetSize(uint_fast32_t& x, uint_fast32_t& y) const noexcept;
		[[nodiscard]] const std::vector<unsigned char> GetData() const;
		[[nodiscard]] VulkanTextureImage* GetTextureImage() const noexcept;
		[[nodiscard]] int GetColorFormat() const noexcept;
	};
} // namespace Engine::Rendering
