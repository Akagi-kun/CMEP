#pragma once

#include "Asset.hpp"
#include "InternalEngineObject.hpp"

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
		std::unordered_map<std::string, std::string> info;
		std::unordered_map<int, std::shared_ptr<Texture>> pages;
		std::unordered_map<int, FontChar> chars;
		unsigned int char_count = 0;
	};

	class Font final : public InternalEngineObject, public Asset
	{
	public:
		using InternalEngineObject::InternalEngineObject;
		~Font();

		void Init(std::unique_ptr<FontData> init_data);

		FontChar* GetChar(char character_id);
		std::shared_ptr<Texture> GetPageTexture(int page);
		std::string* GetFontInfoParameter(const std::string& name);

	private:
		std::unique_ptr<FontData> data;
	};
} // namespace Engine::Rendering
