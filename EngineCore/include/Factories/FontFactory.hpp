#pragma once

#include "Assets/Font.hpp"

#include "InternalEngineObject.hpp"

namespace Engine::Factories
{
	enum class BmFontLineType : uint8_t
	{
		MIN_ENUM = 0x00,
		INFO	 = 2,
		CHAR	 = 4,
		COMMON	 = 6,
		PAGE	 = 8,
		CHARS	 = 10,
		MAX_ENUM = 0xFF
	};

	class FontFactory : public InternalEngineObject
	{
	private:
		void ParseBmfont(std::unique_ptr<Rendering::FontData>& font, std::ifstream& font_file);
		void ParseBmfontLine(
			std::unique_ptr<Rendering::FontData>& font,
			BmFontLineType line_type,
			std::stringstream& line_stream
		);
		void ParseBmfontEntryPage(std::unique_ptr<Rendering::FontData>& font, std::stringstream& line_stream);

	public:
		using InternalEngineObject::InternalEngineObject;

		std::shared_ptr<Rendering::Font> InitBMFont(const std::string& font_path);
	};
} // namespace Engine::Factories
