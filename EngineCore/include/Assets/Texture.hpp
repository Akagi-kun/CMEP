#pragma once

#include "Rendering/Transform.hpp"
#include "Rendering/Vulkan/exports.hpp"

#include "InternalEngineObject.hpp"

#include <memory>
#include <type_traits>
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
		std::vector<unsigned char> data;
		ImageSize size;
		int color_fmt						= 4;
		Vulkan::SampledImage* texture_image = nullptr;
	};

	class Texture final : public InternalEngineObject
	{
	public:
		Texture(Engine* with_engine, std::unique_ptr<TextureData> init_data);
		~Texture();

		Texture(const Texture&) = delete;

		[[nodiscard]] ImageSize GetSize() const
		{
			return this->data->size;
		}

		[[nodiscard]] std::vector<unsigned char>& GetData() const
		{
			return this->data->data;
		}

		[[nodiscard]] Vulkan::SampledImage* GetTextureImage() const
		{
			return this->data->texture_image;
		}

		[[nodiscard]] int GetColorFormat() const
		{
			return this->data->color_fmt;
		}

	private:
		std::unique_ptr<TextureData> data;
	};

	static_assert(!std::is_copy_constructible<Texture>());

} // namespace Engine::Rendering
