#include "Factories/FontFactory.hpp"

#include "Assets/Font.hpp"
#include "Assets/Texture.hpp"

#include "Logging/Logging.hpp"

#include "Detail/KVPairHelper.hpp"
#include "Exception.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>

namespace Engine::Factories
{
#pragma region Static

	namespace
	{
		std::tuple<std::string, std::string> getNextKVPair(std::stringstream& from_stream)
		{
			return Detail::splitKVPair(Detail::streamGetNextToken(from_stream), "=");
		}

		void parseBmfontEntryChar(
			std::unique_ptr<Rendering::FontData>& font,
			std::stringstream&					  line_stream
		)
		{
			// When reading the code in this function,
			// take care to not die from pain as you see the horrible code I have written here.
			// It is designed to be extensible and reduce performance penalty at runtime

			std::unordered_map<std::string, size_t> struct_offsets = {
				{"x", offsetof(Rendering::FontChar, x)},
				{"y", offsetof(Rendering::FontChar, y)},
				{"width", offsetof(Rendering::FontChar, width)},
				{"height", offsetof(Rendering::FontChar, height)},
				{"xoffset", offsetof(Rendering::FontChar, xoffset)},
				{"yoffset", offsetof(Rendering::FontChar, yoffset)},
				{"xadvance", offsetof(Rendering::FontChar, xadvance)},
				{"page", offsetof(Rendering::FontChar, page)},
				{"channel", offsetof(Rendering::FontChar, channel)},
			};

			int					fchar_id = -1;
			Rendering::FontChar fchar{};

			// Using bytes instead of int is safer here
			// because the struct may have padding
			//
			// Treat fchar as bytes in memory
			auto* fchar_memory = reinterpret_cast<uint8_t*>(&fchar);

			while (!line_stream.eof())
			{
				auto [key, value] = getNextKVPair(line_stream);

				// First try finding the key in the offset table
				auto result_offset = struct_offsets.find(key);
				if (result_offset != struct_offsets.end())
				{
					// Offset in memory (pointing to the member of struct)
					int* fchar_entry_ptr = reinterpret_cast<Rendering::FontChar::value_t*>(
						&fchar_memory[result_offset->second]
					);

					// Finally fill the member of the struct
					(*fchar_entry_ptr) = std::stoi(value);
				}
				else if (key == "id")
				{
					fchar_id = std::stoi(value);
				}
			}

			font->chars.emplace(fchar_id, fchar);
		}
	} // namespace

#pragma endregion

#pragma region Public

	std::shared_ptr<Rendering::Font> FontFactory::createFont(
		const std::filesystem::path& font_path,
		const pageload_callback_t&	 opt_callback
	)
	{
		std::ifstream font_file(font_path);

		EXCEPTION_ASSERT(
			font_file.is_open(),
			std::format("font file '{}' could not be opened", font_path.lexically_normal().string())
		);

		this->logger->logSingle<decltype(this)>(
			Logging::LogLevel::VerboseDebug,
			"Loading file '{}'",
			font_path.lexically_normal().string()
		);

		std::unique_ptr<Rendering::FontData> font_data =
			parseBmfont(font_path, font_file, opt_callback);

		this->logger->logSingle<decltype(this)>(
			Logging::LogLevel::Debug,
			"File '{}' loaded successfully",
			font_path.lexically_normal().string()
		);

		font_file.close();

		return std::make_shared<Rendering::Font>(owner_engine, std::move(font_data));
	}

#pragma region Private

	std::unique_ptr<Rendering::FontData> FontFactory::parseBmfont(
		const std::filesystem::path& font_path,
		std::ifstream&				 font_file,
		const pageload_callback_t&	 pageload_cb
	)
	{
		std::unique_ptr<Rendering::FontData> font = std::make_unique<Rendering::FontData>();

		static constexpr size_t		  buffer_size = 255;
		std::array<char, buffer_size> data{};

		while (!font_file.eof())
		{
			data.fill(0);

			// Get a line from file
			font_file.getline(data.data(), data.size());

			std::stringstream line_data(data.data());

			// parse line
			while (!line_data.eof())
			{
				std::string line_type_str;
				line_data >> line_type_str;

				static const std::unordered_map<std::string, BmFontLineType> entry_type_val = {
					{"info", BmFontLineType::INFO},
					{"char", BmFontLineType::CHAR},
					{"common", BmFontLineType::COMMON},
					{"page", BmFontLineType::PAGE},
					{"chars", BmFontLineType::CHARS},
				};

				auto result = entry_type_val.find(line_type_str);

				if (result != entry_type_val.end())
				{
					// Read in the rest of the line
					std::stringstream line_remainder;
					line_data >> line_remainder.rdbuf();

					parseBmfontLine(font_path, font, result->second, line_remainder, pageload_cb);
				}
			}
		}

		return font;
	}

	void FontFactory::parseBmfontEntryPage(
		std::filesystem::path				  font_path,
		std::unique_ptr<Rendering::FontData>& font,
		std::stringstream&					  line_stream,
		const pageload_callback_t&			  pageload_cb
	)
	{
		int					  page_idx = -1;
		std::filesystem::path page_path;

		do
		{
			auto [key, value] = getNextKVPair(line_stream);

			if (key == "file")
			{
				// Remove quotes around filename
				assert(value.size() > 2);
				if (value.front() == '"' && value.back() == '"')
				{
					value = value.substr(1, value.size() - 2);
				}

				page_path = value;
			}
			else if (key == "id")
			{
				page_idx = std::stoi(value);
			}
		} while ((page_idx == -1) || page_path.empty());

		this->logger->logSingle<decltype(this)>(
			Logging::LogLevel::VerboseDebug,
			"Font page index {} is '{}'",
			page_idx,
			page_path.lexically_normal().string()
		);

		std::filesystem::path asset_path = font_path.remove_filename() / page_path;

		std::shared_ptr<Rendering::Texture> texture;
		try
		{
			texture = pageload_cb(asset_path);
		}
		catch (...)
		{
			std::throw_with_nested(ENGINE_EXCEPTION(
				std::format("Exception trying to load font page '{}'", asset_path.string())
			));
		}

		// Add page and it's texture to map
		font->pages.insert(
			std::pair<int, std::shared_ptr<Rendering::Texture>>(page_idx, std::move(texture))
		);
	}

	void FontFactory::parseBmfontLine(
		const std::filesystem::path&		  font_path,
		std::unique_ptr<Rendering::FontData>& font,
		const BmFontLineType				  line_type,
		std::stringstream&					  line_stream,
		const pageload_callback_t&			  pageload_cb
	)
	{
		switch (line_type)
		{
			case BmFontLineType::INFO:
			case BmFontLineType::COMMON:
			{
				// Add every entry of the line
				while (!line_stream.eof())
				{
					auto [key, value] = getNextKVPair(line_stream);
					font->info.emplace(key, value);
				}

				break;
			}
			case BmFontLineType::CHARS:
			{
				// chars has only a single entry (the count of chars), no loop required
				auto [key, value] = getNextKVPair(line_stream);
				font->char_count  = static_cast<unsigned int>(std::stoi(value));

				break;
			}
			case BmFontLineType::CHAR:
			{
				parseBmfontEntryChar(font, line_stream);
				break;
			}
			case BmFontLineType::PAGE:
			{
				try
				{
					parseBmfontEntryPage(font_path, font, line_stream, pageload_cb);
					return;
				}
				catch (...)
				{
					std::throw_with_nested(ENGINE_EXCEPTION("Could not initialize a Font page!"));
				}
			}
			default:
			{
				break;
			}
		}
	}
} // namespace Engine::Factories
