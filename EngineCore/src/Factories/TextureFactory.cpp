#include "Assets/Texture.hpp"
#include "Rendering/Transform.hpp"
#include "Rendering/Vulkan/backend.hpp"

#include "Logging/Logging.hpp"

#include "Exception.hpp"
#include "InternalEngineObject.hpp"

#include <cassert>
#include <exception>
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
	/**
	 * Arbitrary limit to texture sizes, Vulkan may not like very large textures.
	 * This may be completely unnecessary
	 */
	static constexpr size_t texture_size_limit = 0x2fff;

	std::shared_ptr<Rendering::Texture> TextureFactory::createTexture(
		const std::filesystem::path& path,
		vk::Filter					 filtering,
		vk::SamplerAddressMode		 sampler_address_mode
	)
	{
		if (!std::filesystem::exists(path))
		{
			throw ENGINE_EXCEPTION(std::format(
				"Cannot initialize a texture from a nonexistent path! Path: '{}'",
				path.lexically_normal().string()
			));
		}

		this->logger->simpleLog<decltype(this)>(
			Logging::LogLevel::VerboseDebug,
			"Initializing texture from file '%s'",
			path.lexically_normal().string().c_str()
		);

		std::vector<unsigned char>				data;
		std::unique_ptr<Rendering::TextureData> texture_data =
			std::make_unique<Rendering::TextureData>();

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

		if (error != 0 || 0 >= size.x || size.x >= texture_size_limit || 0 >= size.y ||
			size.y >= texture_size_limit)
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

		createTextureInternal(texture_data, std::move(data), 4, filtering, sampler_address_mode, size);

		std::shared_ptr<Rendering::Texture> texture =
			std::make_shared<Rendering::Texture>(owner_engine, std::move(texture_data));

		return texture;
	}

	int TextureFactory::createTextureInternal(
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

		auto* logical_device = vk_instance->getLogicalDevice();

		Rendering::Vulkan::StagingBuffer staging_buffer(
			logical_device,
			vk_instance->getGraphicMemoryAllocator(),
			raw_data.data(),
			memory_size
		);

		texture_data->image = new Rendering::Vulkan::ViewedImage(
			logical_device,
			vk_instance->getGraphicMemoryAllocator(),
			{size.x, size.y},
			vk::SampleCountFlagBits::e1,
			vk::Format::eR8G8B8A8Srgb,
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			vk::ImageAspectFlagBits::eColor
		);

		texture_data->sampler = new Rendering::Vulkan::Sampler(
			logical_device,
			filtering,
			sampler_address_mode,
			vk_instance->getPhysicalDevice()->getLimits().maxSamplerAnisotropy
		);

		try
		{
			auto command_buffer = vk_instance->getCommandPool()->constructCommandBuffer();

			command_buffer.beginOneTime();

			// Transfer image layout to compatible with transfers
			texture_data->image->transitionImageLayout(
				command_buffer,
				vk::ImageLayout::eTransferDstOptimal
			);

			command_buffer.copyBufferImage(&staging_buffer, texture_data->image);

			// Transfer image layout to compatible with rendering
			texture_data->image->transitionImageLayout(
				command_buffer,
				vk::ImageLayout::eShaderReadOnlyOptimal
			);

			command_buffer.end();

			command_buffer.queueSubmit(logical_device->getGraphicsQueue());
		}
		catch (...)
		{
			std::throw_with_nested(
				ENGINE_EXCEPTION("Exception caught trying to fill and transition texture")
			);
		}

		return 0;
	}
} // namespace Engine::Factories
