#pragma once

#include "Assets/Font.hpp"

#include "InternalEngineObject.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <sstream>

namespace Engine::Factories
{
	class FontFactory : public InternalEngineObject
	{
	public:
		using InternalEngineObject::InternalEngineObject;

		using pageload_callback_t =
			std::function<Rendering::FontData::page_t(const std::filesystem::path&)>;

		std::shared_ptr<Rendering::Font> createFont(
			const std::filesystem::path& font_path,
			const pageload_callback_t&	 opt_callback
		);

	private:
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

		std::unique_ptr<Rendering::FontData> parseBmfont(
			const std::filesystem::path& font_path,
			std::ifstream&				 font_file,
			const pageload_callback_t&	 pageload_cb
		);

		/**
		 * @todo Make library for these
		 */
		void parseBmfontLine(
			const std::filesystem::path&		  font_path,
			std::unique_ptr<Rendering::FontData>& font,
			BmFontLineType						  line_type,
			std::stringstream&					  line_stream,
			const pageload_callback_t&			  pageload_cb
		);

		void parseBmfontEntryPage(
			std::filesystem::path				  font_path,
			std::unique_ptr<Rendering::FontData>& font,
			std::stringstream&					  line_stream,
			const pageload_callback_t&			  pageload_cb
		);
	};
} // namespace Engine::Factories
