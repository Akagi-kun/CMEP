#pragma once

#include "InternalEngineObject.hpp"
#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"

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
		unsigned int x = 0, y = 0;
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
		Texture();
		~Texture();

		void Init(std::unique_ptr<TextureData> init_data);

		// void UsePremadeStagingBuffer(VulkanBuffer* staging_buffer);
		/*
				int InitRaw(std::vector<unsigned char> raw_data, int color_format, unsigned int xsize, unsigned int
		   ysize); int InitFile(Texture_InitFiletype filetype, std::string path, unsigned int sizex = 0, unsigned int
		   sizey = 0);
		 */
		void GetSize(unsigned int& x, unsigned int& y) const noexcept;
		const std::vector<unsigned char> GetData() const;
		VulkanTextureImage* GetTextureImage() const noexcept;
		int GetColorFormat() const noexcept;
	};
} // namespace Engine::Rendering
