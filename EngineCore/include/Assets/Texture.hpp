#pragma once

#include "Rendering/Transform.hpp"
#include "Rendering/Vulkan/Wrappers/SampledImage.hpp"

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
		std::vector<unsigned char> data;
		TextureSize size;
		int color_fmt						= 4;
		Vulkan::SampledImage* texture_image = nullptr;
	};

	class Texture final : public InternalEngineObject
	{
	private:
		std::unique_ptr<TextureData> data;

	public:
		Texture(Engine* with_engine, std::unique_ptr<TextureData> init_data);
		~Texture();

		void GetSize(TextureSize& size) const noexcept
		{
			size = this->data->size;
		}

		[[nodiscard]] std::vector<unsigned char>& GetData() const
		{
			return this->data->data;
		}

		[[nodiscard]] Vulkan::SampledImage* GetTextureImage() const noexcept
		{
			return this->data->texture_image;
		}

		[[nodiscard]] int GetColorFormat() const noexcept
		{
			return this->data->color_fmt;
		}
	};
} // namespace Engine::Rendering
