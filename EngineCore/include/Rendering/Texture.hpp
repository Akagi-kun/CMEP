#pragma once

#include <memory>
#include <string>
#include <vector>

#include "VulkanRenderingEngine.hpp"

namespace Engine::Rendering
{
	typedef enum class Texture_InitFiletypeEnum
	{
		FILE_PNG = 3
	} Texture_InitFiletype;

	class __declspec(dllexport) Texture final
	{
	private:
		std::vector<unsigned char> data;
		unsigned int x = 0, y = 0;
		int color_fmt = 4;
		unsigned int texture = 0;

		VulkanBuffer* staging_buffer = nullptr;
		bool managedStagingBuffer = false;
		VulkanTextureImage* textureImage = nullptr;

	public:
		Texture();
		~Texture();

		void UsePremadeStagingBuffer(VulkanBuffer* staging_buffer);

		int InitRaw(std::vector<unsigned char> raw_data, int color_format, unsigned int xsize, unsigned int ysize);
		int InitFile(Texture_InitFiletype filetype, std::string path, unsigned int sizex = 0, unsigned int sizey = 0);

		void GetSize(unsigned int& x, unsigned int& y) const noexcept;
		const std::vector<unsigned char> GetData() const noexcept;
		unsigned int GetTexture() const noexcept;
		VulkanTextureImage* GetTextureImage() const noexcept;
		int GetColorFormat() const noexcept;
	};
}