#pragma once

#include "Asset.hpp"
#include "InternalEngineObject.hpp"

#include <optional>
#include <unordered_map>

namespace Engine::Rendering
{
	// TODO: Remove
	class Texture;

	struct FontChar
	{
		using value_t = int;

		value_t x;
		value_t y;
		value_t width;
		value_t height;
		value_t xoffset;
		value_t yoffset;
		value_t xadvance;
		value_t page;
		value_t channel;
	};

	struct FontData
	{
		using page_t = std::shared_ptr<Texture>;

		std::unordered_map<std::string, std::string> info;
		std::unordered_map<int, page_t> pages;
		std::unordered_map<int, FontChar> chars;
		unsigned int char_count = 0;
	};

	class Font final : public InternalEngineObject, public Asset
	{
	public:
		using InternalEngineObject::InternalEngineObject;
		~Font();

		void Init(std::unique_ptr<FontData> init_data);

		[[nodiscard]] std::shared_ptr<Texture> GetPageTexture(int page);
		[[nodiscard]] std::shared_ptr<const Texture> GetPageTexture(int page) const;

		[[nodiscard]] const FontChar* GetChar(char character_id) const;

		[[nodiscard]] std::optional<std::string> GetFontInfoParameter(const std::string& name
		) const;

	private:
		std::unique_ptr<FontData> data;
	};
} // namespace Engine::Rendering
