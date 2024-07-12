#include "Factories/FontFactory.hpp"

#include "Assets/AssetManager.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_FONT_FACTORY
#include "Logging/LoggingPrefix.hpp"

namespace Engine::Factories
{
	std::shared_ptr<Rendering::Font> FontFactory::InitBMFont(const std::string& font_path)
	{
		std::shared_ptr<Rendering::Font> font = std::make_shared<Rendering::Font>(this->owner_engine);

		std::ifstream font_file(font_path);

		if (!font_file.is_open())
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Error,
				LOGPFX_CURRENT "FontFile %s unexpectedly not open",
				font_path.c_str()
			);
			return nullptr;
		}

		this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Loading file %s", font_path.c_str());

		std::unique_ptr<Rendering::FontData> font_data = std::make_unique<Rendering::FontData>();
		this->ParseBmfont(font_data, font_file);

		this->logger
			->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "File %s loaded successfully", font_path.c_str());

		font_file.close();

		font->Init(std::move(font_data));

		return font;
	}

	static std::string ExtractEntry(std::stringstream& from_sstream)
	{
		std::string entry;

		from_sstream >> entry;

		return entry;
	}

	static std::tuple<std::string, std::string> ParseKeyValuePair(
		const std::string& from_string,
		const std::string& delimiter
	)
	{
		// Get position of delimiter in entry
		const size_t delimiter_begin = from_string.find(delimiter);
		const size_t delimiter_end	 = delimiter_begin + delimiter.size();

		// Get the key and value
		std::string key	  = from_string.substr(0, delimiter_begin);
		std::string value = from_string.substr(delimiter_end, from_string.size() - delimiter_end);

		return {key, value};
	}

	void FontFactory::ParseBmfont(std::unique_ptr<Rendering::FontData>& font, std::ifstream& font_file)
	{
		static constexpr size_t buffer_size = 255;
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

					this->ParseBmfontLine(font, result->second, line_remainder);
				}
			}
		}
	}

	static void HandleBmfontEntryChar(std::unique_ptr<Rendering::FontData>& font, std::stringstream& data)
	{
		// When reading the code in this function,
		// take care to not die from pain as you see the horrible code I have written here.
		// It seemed like a good idea at first but now I realize the mistakes of my old self.

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

		int fchar_id = -1;

		// Abuse a union to stuff data into FontChar without reinterpret_cast
		union InternalFontChar {
			Rendering::FontChar formatted_data;
			int internal_data[sizeof(formatted_data) / sizeof(int)];
		} fchar;

		while (!data.eof())
		{
			// Get next key and value pair
			auto [key, value] = ParseKeyValuePair(ExtractEntry(data), "=");

			// First try finding the key in the offset table
			auto result_offset = struct_offsets.find(key);
			if (result_offset != struct_offsets.end())
			{
				// Get array index from struct offset
				size_t array_index = result_offset->second / sizeof(int);

				fchar.internal_data[array_index] = std::stoi(value);
			}
			else if (key == "id")
			{
				fchar_id = std::stoi(value);
			}
		}

		font->chars.emplace(fchar_id, fchar.formatted_data);
	}

	void FontFactory::ParseBmfontLine(
		std::unique_ptr<Rendering::FontData>& font,
		BmFontLineType type,
		std::stringstream& data
	)
	{
		std::string key;
		std::string value;

		switch (type)
		{
			case BmFontLineType::INFO:
			case BmFontLineType::COMMON:
			{
				// Add every entry of the line
				while (!data.eof())
				{
					tie(key, value) = ParseKeyValuePair(ExtractEntry(data), "=");
					font->info.emplace(key, value);
				}

				break;
			}
			case BmFontLineType::CHARS:
			{
				// chars has only a single entry (the count of chars), no loop required
				tie(key, value)	 = ParseKeyValuePair(ExtractEntry(data), "=");
				font->char_count = static_cast<unsigned int>(std::stoi(value));

				break;
			}
			case BmFontLineType::CHAR:
			{
				HandleBmfontEntryChar(font, data);
				break;
			}
			case BmFontLineType::PAGE:
			{
				int page_idx = -1;
				std::filesystem::path page_path;

				do
				{
					// Get next key and value pair
					tie(key, value) = ParseKeyValuePair(ExtractEntry(data), "=");

					if (key == "file")
					{
						// Remove quotes around filename
						assert(value.size() > 2);
						value = value.substr(1, value.size() - 2);

						// TODO: Generate a proper path and potentially load the page here
						page_path = value;
					}
					else if (key == "id")
					{
						page_idx = std::stoi(value);
					}
				} while ((page_idx == -1) || page_path.empty());

				assert(page_idx != -1);
				assert(!page_path.empty());

				this->logger->SimpleLog(
					Logging::LogLevel::Debug3,
					LOGPFX_CURRENT "Font page index %u is %s",
					page_idx,
					page_path.c_str()
				);

				std::shared_ptr<Rendering::Texture> texture{};

				try
				{
					auto asset_manager = this->owner_engine->GetAssetManager();

					if (auto locked_asset_manager = asset_manager.lock())
					{
						texture = locked_asset_manager->GetTexture(page_path.string());
						assert(texture != nullptr && "GetTexture resulted in nullptr!");

						// Add page and it's texture to map
						font->pages.insert(
							std::pair<int, std::shared_ptr<Rendering::Texture>>(page_idx, std::move(texture))
						);
						page_idx++;
						return; // break;
					}

					throw std::runtime_error("AssetManager could not be locked!");
				}
				catch (std::exception& e)
				{
					this->logger->SimpleLog(
						Logging::LogLevel::Exception,
						LOGPFX_CURRENT "Could not initialize a Font page texture! e.what(): %s",
						e.what()
					);
					throw;
				}
			}
			default:
			{
				break;
			}
		}
	}
} // namespace Engine::Factories
