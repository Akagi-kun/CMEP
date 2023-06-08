#include <assert.h>
#include <fstream>

#include "Rendering/GLCommon.hpp"
#include "Rendering/lodepng/lodepng.hpp"
#include "Rendering/Texture.hpp"
#include "Logging/Logging.hpp"

namespace Engine::Rendering
{
	Texture::Texture() noexcept {} // Constructor
	Texture::Texture(const Texture& other) noexcept // Copy constructor
	{
		unsigned int x = 0, y = 0;
		other.GetSize(x, y);
		this->InitRaw(other.GetData(), other.GetColorFormat(), x, y);
	}
	Texture::Texture(const Texture&& other) noexcept // Move constructor
	{
		other.GetSize(this->x, this->y);
		this->data = other.GetData();
		this->texture = other.GetTexture();
		this->color_fmt = other.GetColorFormat();
	}
	Texture& Texture::operator=(const Texture& other) noexcept // Copy assignment
	{
		if (this == &other)
		{
			return *this;
		}
		unsigned int x = 0, y = 0;
		other.GetSize(x, y);
		this->InitRaw(other.GetData(), other.GetColorFormat(), x, y);
		return *this;
	}
	Texture& Texture::operator=(const Texture&& other) noexcept // Move assignment
	{
		other.GetSize(this->x, this->y);
		this->data = other.GetData();
		this->texture = other.GetTexture();
		this->color_fmt = other.GetColorFormat();
		return *this;
	}
	Texture::~Texture() noexcept // Destructor
	{
		glDeleteTextures(1, &this->texture);
	}

	int Texture::InitRaw(std::vector<unsigned char> raw_data, GLenum color_format, unsigned int xsize, unsigned int ysize)
	{
		int channel_count = 0;
		switch (color_format)
		{
		case GL_RED:
			channel_count = 1;
			break;
		case GL_RG:
			channel_count = 2;
			break;
		case GL_BGR:
		case GL_RGB:
			channel_count = 3;
			break;
		case GL_BGRA:
		case GL_RGBA:
			channel_count = 4;
			break;
		}

		//this->data = new unsigned char[(size_t)ysize * xsize  * channel_count];
		//assert(this->data != nullptr);
		//memcpy(this->data, raw_data, (size_t)ysize * xsize * channel_count);
		this->data = raw_data;

		this->color_fmt = color_format;
		this->x = xsize;
		this->y = ysize;

		glCreateTextures(GL_TEXTURE_2D, 1, &this->texture);

		glTextureStorage2D(this->texture, 1, GL_RGBA8, this->x, this->y);
		glTextureSubImage2D(this->texture, 0, 0, 0, this->x, this->y, color_format, GL_UNSIGNED_BYTE, &this->data[0]);

		glTextureParameteri(this->texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(this->texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(this->texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(this->texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		return 0;
	}

	int Texture::InitFile(Texture_InitFiletype filetype, std::string path, unsigned int sizex, unsigned int sizey)
	{
		FILE* file = nullptr;
		if ((file = fopen(path.c_str(), "rb")) == NULL)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Exception, "File %s could not be found, initializing texture not possible!", path.c_str());
			exit(1);
		}

		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2, "Initializing texture from file %s", path.c_str());

		// Get size
		fseek(file, 0, SEEK_END);
		size_t filesize = ftell(file);
		rewind(file);

		std::vector<unsigned char> data;

		switch (filetype)
		{
			case Texture_InitFiletype::FILE_PNG:
			{
				std::vector<unsigned char> pixels;

				unsigned error = lodepng::decode(pixels, sizex, sizey, path.c_str());

				Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug1, "Decoded png file %s width %u height %u", path.c_str(), sizex, sizey);

				fclose(file);

				this->InitRaw(pixels, GL_RGBA, sizex, sizey);

				return 0;
			}
		}
		fclose(file);

		if (sizex != 0 || sizey != 0)
		{
			this->InitRaw(data, GL_RGB, sizex, sizey);
		}
		else
		{
			return 1;
		}

		return 0;
	}
	
	void Texture::GetSize(unsigned int& x, unsigned int& y) const noexcept
	{
		x = this->x;
		y = this->y;
	}

	const std::vector<unsigned char> Texture::GetData() const noexcept
	{
		return this->data;
	}
	
	GLuint Texture::GetTexture() const noexcept
	{
		return this->texture;
	}

	GLenum Texture::GetColorFormat() const noexcept
	{
		return this->color_fmt;
	}
}