#include "Factories/FontFactory.hpp"

#include "Assets/AssetManager.hpp"

#include "PlatformIndependentUtils.hpp"
#include <cstdint>

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_FONT_FACTORY
#include "Logging/LoggingPrefix.hpp"

#include <cstring>
#include <fstream>

namespace Engine::Factories
{
	FontFactory::FontFactory(AssetManager* manager) : asset_manager(manager)
	{
	}

	std::shared_ptr<Rendering::Font> FontFactory::InitBMFont(std::string fontPath)
	{
		std::shared_ptr<Rendering::Font> font = std::make_shared<Rendering::Font>();
		font->UpdateOwnerEngine(this->owner_engine);
		font->UpdateHeldLogger(this->logger);

		std::unique_ptr<Rendering::FontData> font_data = std::make_unique<Rendering::FontData>();

		std::ifstream font_file(fontPath);

		if (!font_file.is_open())
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Error, LOGPFX_CURRENT "FontFile %s unexpectedly not open", fontPath.c_str()
			);
			return nullptr;
		}

		this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Loading file %s", fontPath.c_str());
		// Evaluate it
		this->EvalBmfont(font_data, font_file);

		this->logger->SimpleLog(
			Logging::LogLevel::Debug2, LOGPFX_CURRENT "File %s loaded successfully", fontPath.c_str()
		);

		font_file.close();

		font->Init(std::move(font_data));

		return font;
	}

	void FontFactory::EvalBmfont(std::unique_ptr<Rendering::FontData>& font, std::ifstream& fontFile)
	{
		static constexpr uint_fast16_t entry_buffer_limit = 16;
		static const uint_fast16_t buffer_limit = 255;

		uint_fast16_t cur_offset = 0;
		char cur_data[entry_buffer_limit] = {};
		char* data = new char[buffer_limit + 1];

		assert(data);

		while (fontFile.eof() == 0)
		{
			memset(data, 0, buffer_limit);

			// Get a line from file
			fontFile.getline(data, buffer_limit);

			// Remove trailing newline
			data[strcspn(data, "\n")] = 0;

			// Consume one entry at a time
			// entries separated by a space character
			// entry size limited
			memset(cur_data, 0, entry_buffer_limit);
			(void)sscanf(data, "%s %n", cur_data, &cur_offset);

			if (strnlen(cur_data, ARRAY_SIZEOF(cur_data)) == 0)
			{
				break;
			}

			assert(0 < cur_offset && cur_offset < buffer_limit);

			static constexpr char info_str[] = "info";
			static constexpr char char_str[] = "char";
			static constexpr char common_str[] = "common";
			static constexpr char page_str[] = "page";
			static constexpr char chars_str[] = "chars";

			// Send EvalBmfontLine the correct type value
			if (strncmp(info_str, cur_data, sizeof(info_str) - 1) == 0)
			{
				this->EvalBmfontLine(font, 0, &data[cur_offset]);
			}
			else if (strncmp(char_str, cur_data, sizeof(char_str) - 1) == 0)
			{
				this->EvalBmfontLine(font, 1, &data[cur_offset]);
			}
			else if (strncmp(common_str, cur_data, sizeof(common_str) - 1) == 0)
			{
				this->EvalBmfontLine(font, 2, &data[cur_offset]);
			}
			else if (strncmp(page_str, cur_data, sizeof(page_str) - 1) == 0)
			{
				this->EvalBmfontLine(font, 3, &data[cur_offset]);
			}
			else if (strncmp(chars_str, cur_data, sizeof(chars_str) - 1) == 0)
			{
				this->EvalBmfontLine(font, 4, &data[cur_offset]);
			}
			else
			{
				this->logger->SimpleLog(Logging::LogLevel::Warning, LOGPFX_CURRENT "unknown: \"%s\"\n", cur_data);
			}
		}

		delete[] data;
	}

	void FontFactory::EvalBmfontLine(std::unique_ptr<Rendering::FontData>& font, int type, char* data)
	{
		static constexpr uint_fast16_t buffer_len_limit = 63;

		// Temporary storage arrays
		char cur_data[buffer_len_limit + 1] = {};
		char key[buffer_len_limit + 1] = {};
		char value[buffer_len_limit + 1] = {};

		// Read single key=value pairs until end of line
		int cur_offset = 0;
		while (sscanf(data, "%s %n", cur_data, &cur_offset) == 1)
		{
			data += cur_offset;
			cur_offset = 0;

			// Get the key and value
			(void)sscanf(cur_data, "%[^=]=%s", &key[0], &value);

			switch (type)
			{
				case 0: // info
				case 2: // common
				{
					font->info.emplace(key, value);
					break;
				}
				case 1: // char
				{
					// Collect keys and values
					static std::unordered_map<std::string, std::string> pairs;
					pairs.emplace(key, value);

					// Last key should always be chnl
					if (strncmp("chnl", key, 4) == 0)
					{
						assert(pairs.find("id") != pairs.end());
						assert(pairs.find("x") != pairs.end());
						assert(pairs.find("y") != pairs.end());
						assert(pairs.find("width") != pairs.end());
						assert(pairs.find("height") != pairs.end());
						assert(pairs.find("xoffset") != pairs.end());
						assert(pairs.find("yoffset") != pairs.end());
						assert(pairs.find("xadvance") != pairs.end());
						assert(pairs.find("page") != pairs.end());
						assert(pairs.find("chnl") != pairs.end());

						Rendering::FontChar c = {};
						int id = 0;
						id = std::stoi(pairs.find("id")->second);
						c.x = std::stoi(pairs.find("x")->second);
						c.y = std::stoi(pairs.find("y")->second);
						c.width = std::stoi(pairs.find("width")->second);
						c.height = std::stoi(pairs.find("height")->second);
						c.xoffset = std::stoi(pairs.find("xoffset")->second);
						c.yoffset = std::stoi(pairs.find("yoffset")->second);
						c.xadvance = std::stoi(pairs.find("xadvance")->second);
						c.page = std::stoi(pairs.find("page")->second);
						c.channel = std::stoi(pairs.find("chnl")->second);

						// Place the character into unordered_map
						font->chars.emplace(id, c);

						// Clear the collector unordered_map
						pairs.clear();
					}
					break;
				}
				case 3: // page
				{
					static int page_idx = 0;
					if (strncmp("file", key, 4) == 0)
					{
						// Remove leading and trailing "
						if (value[0] == '"')
						{
							value[strcspn(&value[1], "\"") + 1] = 0;
						}

						std::string whole_filename = std::string(&value[1]);

						this->logger->SimpleLog(
							Logging::LogLevel::Debug3,
							LOGPFX_CURRENT "Font page index %u is %s",
							page_idx,
							whole_filename.c_str()
						);

						std::shared_ptr<Rendering::Texture> texture{};

						try
						{
							if (this->asset_manager != nullptr)
							{
								texture = this->asset_manager->GetTexture(whole_filename);

								if (texture == nullptr)
								{
									throw std::runtime_error("Texture not found! Tried: '" + whole_filename + "'");
								}
								/* if (texture == nullptr)
								{
									this->asset_manager->AddTexture(whole_filename, whole_filename,
								Rendering::Texture_InitFiletype::FILE_PNG); texture =
								this->asset_manager->GetTexture(whole_filename);
								} */
							}
							// The unlikely case
							else
							{
								throw std::runtime_error("Factory has no AssetManager assigned!");
								// this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "A Font is not
								// managed by a AssetManager, this may be unintentional"); texture =
								// std::make_shared<Rendering::Texture>(); if
								// (texture->InitFile(Rendering::Texture_InitFiletype::FILE_PNG, whole_filename.c_str())
								// != 0)
								//{
								//	this->logger->SimpleLog(Logging::LogLevel::Exception, LOGPFX_CURRENT "Failed
								// initializing texture"); 	throw std::runtime_error("Failed initializing texture!");
								// }
							}
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

						// Add page and it's texture to map
						font->pages.insert(
							std::pair<int, std::shared_ptr<Rendering::Texture>>(page_idx, std::move(texture))
						);
						page_idx++;
					}
					break;
				}
				case 4: // chars
				{
					(void)sscanf(value, "%u", &font->char_count);
					break;
				}
				default:
				{
					break;
				}
			}

			memset(cur_data, 0, buffer_len_limit);
		}
	}
} // namespace Engine::Factories
