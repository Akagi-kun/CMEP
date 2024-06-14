#include "Assets/Font.hpp"

#include "Assets/AssetManager.hpp"
#include "Assets/Texture.hpp"

#include "Logging/Logging.hpp"

#include <assert.h>
#include <string>

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_FONT
#include "Logging/LoggingPrefix.hpp"

namespace Engine::Rendering
{
	Font::Font(AssetManager* managed_by)
	{
		this->asset_manager = managed_by;
	}

	Font::~Font()
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Destructor called");

		this->data.reset();
	}

	void Font::Init(std::unique_ptr<FontData> init_data)
	{
		this->data = std::move(init_data);
	}

	FontChar* Font::GetChar(char ch)
	{
		auto find_ret = this->data->chars.find(ch);
		if (find_ret != this->data->chars.end())
		{
			return &find_ret->second;
		}
		return nullptr;
	}

	std::shared_ptr<Texture> Font::GetPageTexture(int page)
	{
		auto find_ret = this->data->pages.find(page);
		if (find_ret != this->data->pages.end())
		{
			return find_ret->second;
		}
		return nullptr;
	}

	std::string* Font::GetFontInfoParameter(std::string name)
	{
		auto find_ret = this->data->info.find(name);
		if (find_ret != this->data->info.end())
		{
			return &find_ret->second;
		}
		return nullptr;
	}
} // namespace Engine::Rendering
