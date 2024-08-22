#pragma once

#include "Rendering/Transform.hpp"
#include "Rendering/Vulkan/exports.hpp"

#include "InternalEngineObject.hpp"

#include <memory>
#include <vector>

namespace Engine::Rendering
{
	typedef enum class Texture_InitFiletypeEnum : uint8_t
	{
		MIN_ENUM = 0x00,

		FILE_PNG = 3,

		MAX_ENUM = 0xFF
	} Texture_InitFiletype;

	struct TextureData
	{
		ImageSize size;
		int color_fmt = 4;
		std::vector<unsigned char> data;
		Vulkan::SampledImage<Vulkan::ViewedImage>* texture_image = nullptr;
	};

	class Texture final : public InternalEngineObject
	{
	public:
		Texture(Engine* with_engine, std::unique_ptr<TextureData> init_data);
		~Texture();

		// Texture(const Texture&) = delete;

		[[nodiscard]] ImageSize GetSize() const
		{
			return data->size;
		}

		[[nodiscard]] std::vector<unsigned char>& GetData() const
		{
			return data->data;
		}

		[[nodiscard]] auto GetTextureImage() const
		{
			return data->texture_image;
		}

		[[nodiscard]] int GetColorFormat() const
		{
			return data->color_fmt;
		}

	private:
		std::unique_ptr<TextureData> data;
	};

} // namespace Engine::Rendering
