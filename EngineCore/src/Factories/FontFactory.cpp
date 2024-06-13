#include "Factories/FontFactory.hpp"

#include "AssetManager.hpp"
#include "PlatformIndependentUtils.hpp"

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

		std::unique_ptr<Rendering::FontData> fontData = std::make_unique<Rendering::FontData>();

		std::ifstream fontFile(fontPath);

		if (!fontFile.is_open())
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Error, LOGPFX_CURRENT "FontFile %s unexpectedly not open", fontPath.c_str()
			);
			return nullptr;
		}

		this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Loading file %s", fontPath.c_str());
		// Evaluate it
		this->EvalBmfont(fontData, fontFile);

		this->logger->SimpleLog(
			Logging::LogLevel::Debug2, LOGPFX_CURRENT "File %s loaded successfully", fontPath.c_str()
		);

		fontFile.close();

		font->Init(std::move(fontData));

		return font;
	}

	void FontFactory::EvalBmfont(std::unique_ptr<Rendering::FontData>& font, std::ifstream& fontFile)
	{
		int cur_offset = 0;
		char cur_data[16] = {};
		char* data = new char[256];

		assert(data);

		while (fontFile.eof() == 0)
		{
			memset(data, 0, 255);

			// Get a line from file
			fontFile.getline(data, 255);

			// Remove trailing newline
			data[strcspn(data, "\n")] = 0;

			memset(cur_data, 0, 16);
			(void)sscanf(data, "%s %n", cur_data, &cur_offset);

			if (strnlen(cur_data, ARRAY_SIZEOF(cur_data)) == 0)
			{
				break;
			}

			assert(0 < cur_offset && cur_offset < 255);

			// Send EvalBmfontLine the correct type value
			if (strncmp("info", cur_data, 4) == 0)
			{
				this->EvalBmfontLine(font, 0, &data[cur_offset]);
			}
			else if (strncmp("char", cur_data, 4) == 0)
			{
				this->EvalBmfontLine(font, 1, &data[cur_offset]);
			}
			else if (strncmp("common", cur_data, 6) == 0)
			{
				this->EvalBmfontLine(font, 2, &data[cur_offset]);
			}
			else if (strncmp("page", cur_data, 4) == 0)
			{
				this->EvalBmfontLine(font, 3, &data[cur_offset]);
			}
			else if (strncmp("chars", cur_data, 5) == 0)
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
		// Temporary storage arrays
		char cur_data[48] = {};
		char key[16] = {};
		char value[32] = {};

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
						id = std::stoi(pairs.find("id")->second.c_str(), nullptr, 10);
						c.x = std::stoi(pairs.find("x")->second.c_str(), nullptr, 10);
						c.y = std::stoi(pairs.find("y")->second.c_str(), nullptr, 10);
						c.width = std::stoi(pairs.find("width")->second.c_str(), nullptr, 10);
						c.height = std::stoi(pairs.find("height")->second.c_str(), nullptr, 10);
						c.xoffset = std::stoi(pairs.find("xoffset")->second.c_str(), nullptr, 10);
						c.yoffset = std::stoi(pairs.find("yoffset")->second.c_str(), nullptr, 10);
						c.xadvance = std::stoi(pairs.find("xadvance")->second.c_str(), nullptr, 10);
						c.page = std::stoi(pairs.find("page")->second.c_str(), nullptr, 10);
						c.channel = std::stoi(pairs.find("chnl")->second.c_str(), nullptr, 10);

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
			}

			memset(cur_data, 0, 48);
		}
	}
} // namespace Engine::Factories
