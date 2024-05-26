#include <assert.h>
#include <fstream>

#pragma warning( push, 2 )
#include "lodepng/lodepng.h"
#pragma warning( pop )


#include "Rendering/Texture.hpp"
#include "Logging/Logging.hpp"

#include "Engine.hpp"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_TEXTURE
#include "Logging/LoggingPrefix.hpp"

namespace Engine::Rendering
{
	Texture::Texture() {}
	Texture::~Texture()
	{
		VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Deleting texture");
		renderer->cleanupVulkanTextureImage(this->textureImage);
		this->textureImage = nullptr;
	}

	void Texture::UsePremadeStagingBuffer(VulkanBuffer* staging_buffer)
	{
		this->staging_buffer = staging_buffer;
		this->managedStagingBuffer = true;
	}

	int Texture::InitRaw(std::vector<unsigned char> raw_data, int color_format, unsigned int xsize, unsigned int ysize)
	{
		int channel_count = 4;
		
		this->data = raw_data;

		this->color_fmt = color_format;
		this->x = xsize;
		this->y = ysize;

		VkDeviceSize memory_size = static_cast<VkDeviceSize>(xsize) * static_cast<VkDeviceSize>(ysize) * channel_count;

		VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		if (!managedStagingBuffer)
		{
			this->staging_buffer = renderer->createVulkanBuffer(static_cast<size_t>(memory_size), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0);
			//this->staging_buffer = renderer->createVulkanStagingBufferPreMapped(memory_size);
		}
		vkMapMemory(renderer->GetLogicalDevice(), this->staging_buffer->allocationInfo.deviceMemory, this->staging_buffer->allocationInfo.offset, this->staging_buffer->allocationInfo.size, 0, &this->staging_buffer->mappedData);

		memcpy(this->staging_buffer->mappedData, raw_data.data(), static_cast<size_t>(memory_size));

		
		if(const auto& vulkanImageFactory = this->owner_engine->GetVulkanImageFactory().lock())
		{
			this->textureImage = vulkanImageFactory->createTextureImage(
				xsize,
				ysize,
				VK_FORMAT_R8G8B8A8_SRGB,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				VK_FILTER_LINEAR, // Filter for both mag and min
				VK_SAMPLER_ADDRESS_MODE_REPEAT // sampler address mode
				);

			// Transfer image layout to one usable by the shader
			vulkanImageFactory->transitionImageLayout(this->textureImage->image->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			renderer->copyVulcanBufferToImage(this->staging_buffer->buffer, this->textureImage->image->image, static_cast<uint32_t>(xsize), static_cast<uint32_t>(ysize));
			vulkanImageFactory->transitionImageLayout(this->textureImage->image->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			// Unmap staging memory and cleanup buffer if we created it here 
			vkUnmapMemory(renderer->GetLogicalDevice(), this->staging_buffer->allocationInfo.deviceMemory);
			if (!managedStagingBuffer)
			{
				renderer->cleanupVulkanBuffer(this->staging_buffer);
			}

			vulkanImageFactory->appendImageViewToTextureImage(this->textureImage);
			renderer->appendVulkanSamplerToVulkanTextureImage(this->textureImage);
		}

		return 0;
	}

	int Texture::InitFile(Texture_InitFiletype filetype, std::string path, unsigned int sizex, unsigned int sizey)
	{
		FILE* file = nullptr;
		if ((file = fopen(path.c_str(), "rb")) == NULL)
		{
			this->logger->SimpleLog(Logging::LogLevel::Exception, LOGPFX_CURRENT "File %s could not be found, initializing texture not possible!", path.c_str());
			throw std::runtime_error("Could not find texture!");
		}

		this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Initializing texture from file %s", path.c_str());

		// Get size
		fseek(file, 0, SEEK_END);
		size_t filesize = ftell(file);
		rewind(file);

		std::vector<unsigned char> data;

		switch (filetype)
		{
			case Texture_InitFiletype::FILE_PNG:
			{
				unsigned int xs, ys;
				unsigned error = lodepng::decode(data, xs, ys, path.c_str());

				this->logger->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "Decoded png file %s width %u height %u", path.c_str(), xs, ys);

				fclose(file);

				assert(0 < xs && xs < 0x3fff);
				assert(0 < ys && ys < 0x3fff);

				this->InitRaw(std::move(data), 4, xs, ys);

				return 0;
			}
		}
		fclose(file);

		if (sizex != 0 || sizey != 0)
		{
			this->InitRaw(std::move(data), 3, sizex, sizey);
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

	const std::vector<unsigned char> Texture::GetData() const
	{
		return this->data;
	}
	
	unsigned int Texture::GetTexture() const noexcept
	{
		return this->texture;
	}

	VulkanTextureImage* Texture::GetTextureImage() const noexcept
	{
		return this->textureImage;
	}
	
	int Texture::GetColorFormat() const noexcept
	{
		return this->color_fmt;
	}
}