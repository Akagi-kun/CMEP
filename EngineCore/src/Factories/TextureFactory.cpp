#include "InternalEngineObject.hpp"

#include <cstddef>
#include <cstdint>
#include <stdexcept>

#pragma warning(push, 2)
#include "lodepng.h"
#pragma warning(pop)

#include "Rendering/Vulkan/exports.hpp"

#include "Factories/TextureFactory.hpp"

#include "Engine.hpp"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_TEXTURE_FACTORY
#include "Logging/LoggingPrefix.hpp" // IWYU pragma: keep

namespace Engine::Factories
{
	static constexpr size_t max_texture_size = 0x2fff;

	std::shared_ptr<Rendering::Texture> TextureFactory::InitFile(
		const std::filesystem::path& path,
		Rendering::Texture_InitFiletype filetype,
		vk::Filter filtering,
		vk::SamplerAddressMode sampler_address_mode
	)
	{
		if (!std::filesystem::exists(path))
		{
			throw std::invalid_argument("Cannot initialize a texture from a nonexistent path!");
		}

		this->logger
			->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Initializing texture from file %s", path.c_str());

		std::vector<unsigned char> data;
		std::unique_ptr<Rendering::TextureData> texture_data = std::make_unique<Rendering::TextureData>();

		switch (filetype)
		{
			case Rendering::Texture_InitFiletype::FILE_PNG:
			{
				Rendering::ImageSize size;
				unsigned int error = lodepng::decode(data, size.x, size.y, path.string());

				if (error != 0 || 0 >= size.x || size.x >= max_texture_size || 0 >= size.y ||
					size.y >= max_texture_size)
				{
					throw std::runtime_error("Failed decoding PNG file!");
				}

				this->logger->SimpleLog(
					Logging::LogLevel::Debug1,
					LOGPFX_CURRENT "Decoded png file %s; width %u; height %u; filter %u",
					path.c_str(),
					size.x,
					size.y,
					filtering
				);

				InitRaw(texture_data, std::move(data), 4, filtering, sampler_address_mode, size);
				break;
			}
			default:
			{
				throw std::runtime_error("Unknown texture filetype passed to TextureFactory!");
			}
		}

		// fclose(file);

		std::shared_ptr<Rendering::Texture> texture =
			std::make_shared<Rendering::Texture>(owner_engine, std::move(texture_data));

		return texture;
	}

	int TextureFactory::InitRaw(
		std::unique_ptr<Rendering::TextureData>& texture_data,
		std::vector<unsigned char> raw_data,
		int color_format,
		vk::Filter filtering,
		vk::SamplerAddressMode sampler_address_mode,
		Rendering::ImageSize size
	)
	{
		uint_fast8_t channel_count = 4;

		texture_data->data		= raw_data;
		texture_data->color_fmt = color_format;
		texture_data->size		= size;

		auto memory_size = static_cast<vk::DeviceSize>(size.x * size.y) * channel_count;

		Rendering::Vulkan::Instance* vk_instance = owner_engine->GetVulkanInstance();
		assert(vk_instance);

		Rendering::Vulkan::Buffer* staging_buffer =
			new Rendering::Vulkan::StagingBuffer(vk_instance, raw_data.data(), memory_size);

		texture_data->texture_image = new Rendering::Vulkan::SampledImage(
			vk_instance,
			{size.x, size.y},
			vk::SampleCountFlagBits::e1,
			vk::Format::eR8G8B8A8Srgb,
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			filtering,			 // Filter for both mag and min
			sampler_address_mode // sampler address mode
		);

		// Transfer image layout to compatible with transfers
		texture_data->texture_image->TransitionImageLayout(vk::ImageLayout::eTransferDstOptimal);

		auto command_buffer = vk_instance->GetCommandPool()->AllocateTemporaryCommandBuffer();

		// auto* command_buffer = vk_instance->GetCommandPool()->AllocateCommandBuffer();

		command_buffer.BufferImageCopy(staging_buffer, texture_data->texture_image);

		// delete command_buffer;
		delete staging_buffer;

		// Transfer image layout to compatible with rendering
		texture_data->texture_image->TransitionImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

		texture_data->texture_image->AddImageView();

		return 0;
	}
} // namespace Engine::Factories
