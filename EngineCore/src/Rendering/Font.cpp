#include <assert.h>
#include <cstdio>
#include <string>
#include <cstring>

#include "Rendering/Texture.hpp"
#include "Logging/Logging.hpp"
#include "Rendering/Font.hpp"
#include "AssetManager.hpp"

namespace Engine::Rendering
{
	Font::Font(AssetManager* managed_by) noexcept
	{
		this->asset_manager = managed_by;
	}

	Font::~Font() noexcept 
	{
	}

	int Font::Init(std::string path) noexcept
	{
		// Opens the bmfont file
		this->fntfile = path;
		//printf("%s\n", path.c_str());
		FILE* file = nullptr;
		if ((file = fopen(path.c_str(), "r")) == NULL)
		{
			assert(0);
			return 1;
		}

		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2, "Loading font file %s", path.c_str());
		// Evaluate it
		this->EvalBmfont(file);
		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2, "Font file %s loaded successfully", path.c_str());

		fclose(file);

		return 0;
	}

	void Font::EvalBmfont(FILE* file) noexcept
	{
		int cur_offset = 0;
		char cur_data[16] = {};
		char* data = new char[256];
		while (feof(file) == 0)
		{
			memset(data, 0, 256);

			// Get a line from file
			fgets(data, 256, file);

			// Remove trailing newline
			data[strcspn(data, "\n")] = 0;

			memset(cur_data, 0, 16);
			(void)sscanf(data, "%s %n", cur_data, &cur_offset);

			if (strnlen(cur_data, 16) == 0)
			{
				break;
			}

			// Send EvalBmfontLine the correct type value
			if (strncmp("info", cur_data, 4) == 0)
			{
				this->EvalBmfontLine(0, &data[cur_offset]);
			}
			else if (strncmp("char", cur_data, 4) == 0)
			{
				this->EvalBmfontLine(1, &data[cur_offset]);
			}
			else if (strncmp("common", cur_data, 6) == 0)
			{
				this->EvalBmfontLine(2, &data[cur_offset]);
			}
			else if (strncmp("page", cur_data, 4) == 0)
			{
				this->EvalBmfontLine(3, &data[cur_offset]);
			}
			else if (strncmp("chars", cur_data, 5) == 0)
			{
				this->EvalBmfontLine(4, &data[cur_offset]);
			}
			else
			{
				Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Warning, "unknown: \"%s\"\n", cur_data);
			}
		}
		delete[] data;
		
	}

	void Font::EvalBmfontLine(int type, char* data) noexcept
	{
		// Temporary storage arrays
		char cur_data[48] = {};
		char key[16] = {};
		char value[32] = {};

		// Read single key=value pairs until end of line
		int cur_offset = 0;
		while (sscanf(data, "%s %n", cur_data, &cur_offset) == 1)
		{
			data += cur_offset; cur_offset = 0;

			// Get the key and value
			(void)sscanf(cur_data, "%[^=]=%s", &key[0], &value);

			switch (type)
			{
			case 0: // info
			case 2: // common
			{
				this->info.emplace(key, value);
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
					FontChar c = {};
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
					this->chars.emplace(id, c);

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

					// bmfont uses local filenames relative to the fnt file
					// Get directory from fnt file path and add our page filename to it
					std::string whole_filename;
					const size_t last_slash_idx = this->fntfile.rfind('/');
					if (std::string::npos != last_slash_idx)
					{
						whole_filename = this->fntfile.substr(0, last_slash_idx);
					}
					whole_filename += "/" + std::string(&value[1]);

					Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug3, "Font page index %u is %s", page_idx, whole_filename.c_str());

					Texture* texture = nullptr;
					if (this->asset_manager == nullptr)
					{
						Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug3, "A Font is not managed by a AssetManager, this may be unintentional");
						texture = new Texture();
						if (texture->InitFile(Texture_InitFiletype::FILE_PNG, whole_filename.c_str()) != 0)
						{
							Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "Failed initializing texture\n");
							assert(0);
						}
					}
					else
					{
						if ((texture = this->asset_manager->GetTexture(whole_filename)) == nullptr)
						{
							this->asset_manager->AddTexture(whole_filename, whole_filename, Texture_InitFiletype::FILE_PNG);
							texture = this->asset_manager->GetTexture(whole_filename);
						}
					}

					// Add page and it's texture to map
					this->pages.insert(std::pair<int, Texture*>(page_idx, texture));
					page_idx++;
				}
				break;
			}
			case 4: // chars
			{
				(void)sscanf(value, "%u", &this->char_count);
				break;
			}
			}

			memset(cur_data, 0, 48);
		}
	}

	FontChar* Font::GetChar(char ch) noexcept
	{
		auto find_ret = this->chars.find(ch);
		if (find_ret != this->chars.end())
		{
			return &find_ret->second;
		}
		return nullptr;
	}

	Texture* Font::GetPageTexture(int page) noexcept
	{
		auto find_ret = this->pages.find(page);
		if (find_ret != this->pages.end())
		{
			return find_ret->second;
		}
		return nullptr;
	}

	std::string* Font::GetFontInfoParameter(std::string name) noexcept
	{
		auto find_ret = this->info.find(name);
		if (find_ret != this->info.end())
		{
			return &find_ret->second;
		}
		return nullptr;
	}
}