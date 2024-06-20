// #include <fstream>

#include <cstdint>
#pragma warning(push, 2)
#include "lodepng.h"
#pragma warning(pop)

#include "Rendering/Vulkan/VulkanImageFactory.hpp"

#include "Factories/TextureFactory.hpp"

#include "Engine.hpp"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_TEXTURE_FACTORY
#include "Logging/LoggingPrefix.hpp"

namespace Engine::Factories
{
	std::shared_ptr<Rendering::Texture> TextureFactory::InitFile(
		const std::string& path,
		Rendering::VulkanBuffer* staging_buffer,
		Rendering::Texture_InitFiletype filetype,
		VkFilter filtering,
		VkSamplerAddressMode sampler_address_mode,
		unsigned int sizex,
		unsigned int sizey
	)
	{
		FILE* file = nullptr;
		if ((file = fopen(path.c_str(), "rb")) == NULL)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Exception,
				LOGPFX_CURRENT "File %s could not be found, initializing texture not possible!",
				path.c_str()
			);
			throw std::runtime_error("Could not find texture!");
		}

		this->logger
			->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Initializing texture from file %s", path.c_str());

		// Get size
		// fseek(file, 0, SEEK_END);
		// size_t filesize = ftell(file);
		// rewind(file);

		std::vector<unsigned char> data;

		std::unique_ptr<Rendering::TextureData> texture_data = std::make_unique<Rendering::TextureData>();

		switch (filetype)
		{
			case Rendering::Texture_InitFiletype::FILE_PNG:
			{
				unsigned int size_x;
				unsigned int size_y;
				unsigned error = lodepng::decode(data, size_x, size_y, path);

				assert(error == 0);

				this->logger->SimpleLog(
					Logging::LogLevel::Debug1,
					LOGPFX_CURRENT "Decoded png file %s; width %u; height %u; filter %u",
					path.c_str(),
					size_x,
					size_y,
					filtering
				);

				assert(0 < size_x && size_x < 0x2fff);
				assert(0 < size_y && size_y < 0x2fff);

				this->InitRaw(
					texture_data,
					staging_buffer,
					std::move(data),
					4,
					filtering,
					sampler_address_mode,
					size_x,
					size_y
				);
				break;
			}
			default:
			{
				throw std::runtime_error("Unknown texture filetype passed to TextureFactory!");
			}
		}
		/*
				if (sizex != 0 || sizey != 0)
				{
					this->InitRaw(std::move(data), 3, sizex, sizey);
				}
				else
				{
					return 1;
				}
		*/

		fclose(file);

		std::shared_ptr<Rendering::Texture> texture = std::make_shared<Rendering::Texture>();
		texture->UpdateOwnerEngine(this->owner_engine);
		texture->UpdateHeldLogger(this->logger);

		texture->Init(std::move(texture_data));

		return texture;
	}

	int TextureFactory::InitRaw(
		std::unique_ptr<Rendering::TextureData>& texture_data,
		Rendering::VulkanBuffer* staging_buffer,
		std::vector<unsigned char> raw_data,
		int color_format,
		VkFilter filtering,
		VkSamplerAddressMode sampler_address_mode,
		unsigned int xsize,
		unsigned int ysize
	)
	{
		uint_fast8_t channel_count = 4;

		texture_data->data		   = raw_data;
		texture_data->color_fmt	   = color_format;
		texture_data->filtering	   = filtering;
		texture_data->address_mode = sampler_address_mode;
		texture_data->x			   = xsize;
		texture_data->y			   = ysize;

		auto memory_size = static_cast<VkDeviceSize>(xsize * ysize * channel_count);

		Rendering::VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		Rendering::VulkanBuffer* used_staging_buffer;

		// If no valid buffer was passed then create one here
		if (staging_buffer == nullptr)
		{
			used_staging_buffer = renderer->CreateVulkanBuffer(
				static_cast<size_t>(memory_size),
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				0
			);
			// this->staging_buffer = renderer->createVulkanStagingBufferPreMapped(memory_size);
		}
		else
		{
			// If it was, use that one
			used_staging_buffer = staging_buffer;
		}

		vkMapMemory(
			renderer->GetLogicalDevice(),
			used_staging_buffer->allocation_info.deviceMemory,
			used_staging_buffer->allocation_info.offset,
			used_staging_buffer->allocation_info.size,
			0,
			&used_staging_buffer->mapped_data
		);

		memcpy(used_staging_buffer->mapped_data, raw_data.data(), static_cast<size_t>(memory_size));

		if (const auto& vulkan_image_factory = this->owner_engine->GetVulkanImageFactory().lock())
		{
			texture_data->texture_image = vulkan_image_factory->CreateTextureImage(
				xsize,
				ysize,
				VK_FORMAT_R8G8B8A8_UNORM,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				filtering,			 // Filter for both mag and min
				sampler_address_mode // sampler address mode
			);

			// Transfer image layout to one usable by the shader
			// TODO: Create utility function for image transfers
			vulkan_image_factory->TransitionImageLayout(
				texture_data->texture_image->image->image,
				VK_FORMAT_R8G8B8A8_UNORM,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
			);
			renderer->CopyVulkanBufferToImage(
				used_staging_buffer->buffer,
				texture_data->texture_image->image->image,
				static_cast<uint32_t>(xsize),
				static_cast<uint32_t>(ysize)
			);
			vulkan_image_factory->TransitionImageLayout(
				texture_data->texture_image->image->image,
				VK_FORMAT_R8G8B8A8_SRGB,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			);

			// Unmap staging memory and cleanup buffer if we created it here
			vkUnmapMemory(renderer->GetLogicalDevice(), used_staging_buffer->allocation_info.deviceMemory);
			if (staging_buffer == nullptr)
			{
				renderer->CleanupVulkanBuffer(used_staging_buffer);
			}

			vulkan_image_factory->AppendImageViewToTextureImage(texture_data->texture_image);
			renderer->AppendVulkanSamplerToVulkanTextureImage(texture_data->texture_image);
		}
		else
		{
			return 1;
		}

		return 0;
	}
} // namespace Engine::Factories
