#include "Assets/Font.hpp"

#include "Assets/AssetManager.hpp"
#include "Assets/Texture.hpp"

#include "Logging/Logging.hpp"

#include <cassert>
#include <string>

namespace Engine::Rendering
{
#pragma region Public

	Font::~Font()
	{
		logger->SimpleLog<decltype(this)>(Logging::LogLevel::VerboseDebug, "Destructor called");

		data.reset();
	}

	void Font::Init(std::unique_ptr<FontData> init_data)
	{
		data = std::move(init_data);
	}

	const FontChar* Font::GetChar(char character_id) const
	{
		auto find_ret = data->chars.find(character_id);
		if (find_ret != data->chars.end())
		{
			return &find_ret->second;
		}
		return nullptr;
	}

	std::shared_ptr<Texture> Font::GetPageTexture(int page)
	{
		auto find_ret = data->pages.find(page);
		if (find_ret != data->pages.end())
		{
			return find_ret->second;
		}
		return nullptr;
	}

	std::shared_ptr<const Texture> Font::GetPageTexture(int page) const
	{
		auto find_ret = data->pages.find(page);
		if (find_ret != data->pages.end())
		{
			return find_ret->second;
		}
		return nullptr;
	}

	std::optional<std::string> Font::GetFontInfoParameter(const std::string& name) const
	{
		auto find_ret = data->info.find(name);
		if (find_ret != data->info.end())
		{
			return find_ret->second;
		}
		return {};
	}

#pragma endregion
} // namespace Engine::Rendering
