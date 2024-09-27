#include "Assets/Texture.hpp"
#include "Rendering/Transform.hpp"
#include "Rendering/Vulkan/backend.hpp"

#include "Logging/Logging.hpp"

#include "Exception.hpp"
#include "InternalEngineObject.hpp"

#include <cassert>
#include <filesystem>
#include <memory>
#include <utility>
#include <vector>

#pragma warning(push, 2)
#include "lodepng.h"
#pragma warning(pop)

#include "Rendering/Vulkan/exports.hpp"

#include "Factories/TextureFactory.hpp"

#include "Engine.hpp"

#include <cstddef>
#include <cstdint>
#include <format>

namespace Engine::Factories
{
	static constexpr size_t max_texture_size = 0x2fff;

	std::shared_ptr<Rendering::Texture> TextureFactory::initFile(
		const std::filesystem::path&	path,
		Rendering::Texture_InitFiletype filetype,
		vk::Filter						filtering,
		vk::SamplerAddressMode			sampler_address_mode
	)
	{
		if (!std::filesystem::exists(path))
		{
			throw ENGINE_EXCEPTION(std::format(
				"Cannot initialize a texture from a nonexistent path! Path: {}",
				path.string()
			));
		}

		this->logger->simpleLog<decltype(this)>(
			Logging::LogLevel::VerboseDebug,
			"Initializing texture from file %s",
			path.string().c_str()
		);

		std::vector<unsigned char>				data;
		std::unique_ptr<Rendering::TextureData> texture_data =
			std::make_unique<Rendering::TextureData>();

		switch (filetype)
		{
			case Rendering::Texture_InitFiletype::FILE_PNG:
			{
				Rendering::ImageSize size;
				unsigned int		 error;

				// lodepng uses references for output
				// this makes it incompatible with ImageSize when defined with
				// different sized integer
				{
					unsigned int size_x;
					unsigned int size_y;

					error = lodepng::decode(data, size_x, size_y, path.string());
					size  = {size_x, size_y};
				}

				ENGINE_EXCEPTION_ON_ASSERT_NOMSG(error == 0)

				if (error != 0 || 0 >= size.x || size.x >= max_texture_size ||
					0 >= size.y || size.y >= max_texture_size)
				{
					throw ENGINE_EXCEPTION("Failed decoding PNG file!");
				}

				this->logger->simpleLog<decltype(this)>(
					Logging::LogLevel::VerboseDebug,
					"Decoded png file %s; width %u; height %u; filter %u",
					path.c_str(),
					size.x,
					size.y,
					filtering
				);

				initRaw(
					texture_data,
					std::move(data),
					4,
					filtering,
					sampler_address_mode,
					size
				);
				break;
			}
			default:
			{
				throw ENGINE_EXCEPTION(
					"Unknown texture filetype passed to TextureFactory!"
				);
			}
		}

		std::shared_ptr<Rendering::Texture> texture =
			std::make_shared<Rendering::Texture>(owner_engine, std::move(texture_data));

		return texture;
	}

	int TextureFactory::initRaw(
		std::unique_ptr<Rendering::TextureData>& texture_data,
		std::vector<unsigned char>				 raw_data,
		int										 color_format,
		vk::Filter								 filtering,
		vk::SamplerAddressMode					 sampler_address_mode,
		Rendering::ImageSize					 size
	)
	{
		uint_fast8_t channel_count = 4;

		texture_data->data		= raw_data;
		texture_data->color_fmt = color_format;
		texture_data->size		= size;

		auto memory_size = static_cast<vk::DeviceSize>(size.x * size.y) * channel_count;

		Rendering::Vulkan::Instance* vk_instance = owner_engine->getVulkanInstance();
		assert(vk_instance);

		Rendering::Vulkan::StagingBuffer staging_buffer(
			vk_instance->getLogicalDevice(),
			vk_instance->getGraphicMemoryAllocator(),
			raw_data.data(),
			memory_size
		);

		texture_data->image = new Rendering::Vulkan::ViewedImage(
			vk_instance,
			{size.x, size.y},
			vk::SampleCountFlagBits::e1,
			vk::Format::eR8G8B8A8Srgb,
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			vk::ImageAspectFlagBits::eColor
		);

		texture_data->sampler = new Rendering::Vulkan::Sampler(
			vk_instance->getLogicalDevice(),
			filtering,
			sampler_address_mode,
			vk_instance->getPhysicalDevice()->getLimits().maxSamplerAnisotropy
		);

		// Transfer image layout to compatible with transfers
		texture_data->image->transitionImageLayout(vk::ImageLayout::eTransferDstOptimal);

		auto command_buffer = vk_instance->getCommandPool()->constructCommandBuffer();

		command_buffer.copyBufferImage(
			vk_instance->getLogicalDevice()->getGraphicsQueue(),
			&staging_buffer,
			texture_data->image
		);

		// Transfer image layout to compatible with rendering
		texture_data->image->transitionImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal
		);

		return 0;
	}
} // namespace Engine::Factories
