#include <cassert>

#pragma warning(push, 2)
#include "lodepng.h"
#pragma warning(pop)

#include "Assets/Texture.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_TEXTURE
#include "Logging/LoggingPrefix.hpp"

namespace Engine::Rendering
{
	Texture::Texture()
	{
	}
	Texture::~Texture()
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Destructor called");

		VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		vkDeviceWaitIdle(renderer->GetLogicalDevice());

		renderer->cleanupVulkanTextureImage(this->data->texture_image);

		this->data.release();
	}
	/*
		void Texture::UsePremadeStagingBuffer(VulkanBuffer* staging_buffer)
		{
			this->staging_buffer = staging_buffer;
			this->managedStagingBuffer = true;
		}
	 */
	void Texture::Init(std::unique_ptr<TextureData> init_data)
	{
		this->data = std::move(init_data);
	}
	void Texture::GetSize(unsigned int& x, unsigned int& y) const noexcept
	{
		x = this->data->x;
		y = this->data->y;
	}

	const std::vector<unsigned char> Texture::GetData() const
	{
		return this->data->data;
	}

	VulkanTextureImage* Texture::GetTextureImage() const noexcept
	{
		return this->data->texture_image;
	}

	int Texture::GetColorFormat() const noexcept
	{
		return this->data->color_fmt;
	}
} // namespace Engine::Rendering
