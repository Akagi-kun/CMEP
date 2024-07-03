#pragma once

#include "Rendering/Vulkan/VulkanTextureImage.hpp"

#include "InternalEngineObject.hpp"

#include <memory>
#include <vector>

namespace Engine::Rendering
{
	class VulkanTextureImage;

	typedef enum class Texture_InitFiletypeEnum : uint8_t
	{
		MIN_ENUM = 0x00,

		FILE_PNG = 3,

		MAX_ENUM = 0xFF
	} Texture_InitFiletype;

	struct TextureData
	{
		std::vector<unsigned char> data;
		uint_fast32_t x = 0;
		uint_fast32_t y = 0;
		int color_fmt	= 4;
		VkFilter filtering;
		VkSamplerAddressMode address_mode;
		VulkanTextureImage* texture_image = nullptr;
	};

	class Texture final : public InternalEngineObject
	{
	private:
		std::unique_ptr<TextureData> data;

	public:
		using InternalEngineObject::InternalEngineObject;
		~Texture();

		void Init(std::unique_ptr<TextureData> init_data);

		void GetSize(uint_fast32_t& x, uint_fast32_t& y) const noexcept;
		[[nodiscard]] const std::vector<unsigned char> GetData() const;
		[[nodiscard]] VulkanTextureImage* GetTextureImage() const noexcept;
		[[nodiscard]] int GetColorFormat() const noexcept;
	};
} // namespace Engine::Rendering
