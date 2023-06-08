#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Rendering/GLCommon.hpp"

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
		GLenum color_fmt = GL_RGB;
		GLuint texture = 0;

	public:
		Texture() noexcept;
		Texture(const Texture& other) noexcept;
		Texture(const Texture&& other) noexcept;
		Texture& operator=(const Texture& other) noexcept;
		Texture& operator=(const Texture&& other) noexcept;
		~Texture() noexcept;

		int InitRaw(std::vector<unsigned char> raw_data, GLenum color_format, unsigned int xsize, unsigned int ysize);
		int InitFile(Texture_InitFiletype filetype, std::string path, unsigned int sizex = 0, unsigned int sizey = 0);

		void GetSize(unsigned int& x, unsigned int& y) const noexcept;
		const std::vector<unsigned char> GetData() const noexcept;
		GLuint GetTexture() const noexcept;
		GLenum GetColorFormat() const noexcept;
	};
}