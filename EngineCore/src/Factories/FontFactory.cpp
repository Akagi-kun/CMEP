#include "Factories/FontFactory.hpp"

#include "Assets/AssetManager.hpp"
#include "Assets/Font.hpp"
#include "Assets/Texture.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"
#include "Exception.hpp"
#include "KVPairHelper.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>

namespace Engine::Factories
{
#pragma region Static

	static std::tuple<std::string, std::string> GetNextKVPair(std::stringstream& from_stream)
	{
		return Utility::SplitKVPair(Utility::StreamGetNextToken(from_stream), "=");
	}

	static void ParseBmfontEntryChar(
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
		auto* fchar_memory = reinterpret_cast<char*>(&fchar);

		while (!line_stream.eof())
		{
			auto [key, value] = GetNextKVPair(line_stream);

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

#pragma endregion

#pragma region Public
	std::shared_ptr<Rendering::Font> FontFactory::CreateFont(
		const std::filesystem::path& font_path,
		const pageload_callback_t&	 opt_callback
	)
	{
		std::ifstream font_file(font_path);

		ENGINE_EXCEPTION_ON_ASSERT(
			font_file.is_open(),
			std::format("font file {} could not be opened", font_path.string())
		)

		this->logger->SimpleLog<decltype(this)>(
			Logging::LogLevel::VerboseDebug,
			"Loading file %s",
			font_path.string().c_str()
		);

		std::unique_ptr<Rendering::FontData> font_data =
			ParseBmfont(font_path, font_file, opt_callback);

		this->logger->SimpleLog<decltype(this)>(
			Logging::LogLevel::Debug,
			"File %s loaded successfully",
			font_path.string().c_str()
		);

		font_file.close();

		return std::make_shared<Rendering::Font>(owner_engine, std::move(font_data));
	}

#pragma region Private

	std::unique_ptr<Rendering::FontData> FontFactory::ParseBmfont(
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

					ParseBmfontLine(font_path, font, result->second, line_remainder, pageload_cb);
				}
			}
		}

		return font;
	}

	void FontFactory::ParseBmfontEntryPage(
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
			auto [key, value] = GetNextKVPair(line_stream);

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

		this->logger->SimpleLog<decltype(this)>(
			Logging::LogLevel::VerboseDebug,
			"Font page index %u is '%s'",
			page_idx,
			page_path.string().c_str()
		);

		auto asset_manager = owner_engine->GetAssetManager();

		if (auto locked_asset_manager = asset_manager.lock())
		{
			std::shared_ptr<Rendering::Texture> texture = locked_asset_manager->GetTexture(
				page_path.string()
			);

			// Try to load the page if asset manager doesnt have it loaded
			if (texture == nullptr)
			{
				std::filesystem::path asset_path = font_path.remove_filename();
				asset_path /= page_path;
				texture = pageload_cb(asset_path);
			}

			// Add page and it's texture to map
			font->pages.insert(
				std::pair<int, std::shared_ptr<Rendering::Texture>>(page_idx, std::move(texture))
			);

			return;
		}

		throw std::runtime_error("AssetManager could not be locked!");
	}

	void FontFactory::ParseBmfontLine(
		const std::filesystem::path&		  font_path,
		std::unique_ptr<Rendering::FontData>& font,
		const BmFontLineType				  line_type,
		std::stringstream&					  line_stream,
		const pageload_callback_t&			  pageload_cb
	)
	{
		std::string key;
		std::string value;

		switch (line_type)
		{
			case BmFontLineType::INFO:
			case BmFontLineType::COMMON:
			{
				// Add every entry of the line
				while (!line_stream.eof())
				{
					tie(key, value
					) = Utility::SplitKVPair(Utility::StreamGetNextToken(line_stream), "=");
					font->info.emplace(key, value);
				}

				break;
			}
			case BmFontLineType::CHARS:
			{
				// chars has only a single entry (the count of chars), no loop required
				tie(key,
					value) = Utility::SplitKVPair(Utility::StreamGetNextToken(line_stream), "=");
				font->char_count = static_cast<unsigned int>(std::stoi(value));

				break;
			}
			case BmFontLineType::CHAR:
			{
				ParseBmfontEntryChar(font, line_stream);
				break;
			}
			case BmFontLineType::PAGE:
			{
				try
				{
					ParseBmfontEntryPage(font_path, font, line_stream, pageload_cb);
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
