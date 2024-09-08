#include "Assets/Font.hpp"

#include "Assets/AssetManager.hpp"
#include "Assets/Texture.hpp"

#include "Logging/Logging.hpp"

#include "Exception.hpp"
#include "InternalEngineObject.hpp"

#include <cassert>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace Engine::Rendering
{
#pragma region Public

	Font::Font(Engine* with_engine, std::unique_ptr<FontData> init_data)
		: InternalEngineObject(with_engine), data(std::move(init_data))
	{
	}

	Font::~Font()
	{
		logger->simpleLog<decltype(this)>(Logging::LogLevel::VerboseDebug, "Destructor called");

		data.reset();
	}

	std::shared_ptr<Texture> Font::getPageTexture(int page)
	{
		auto find_ret = data->pages.find(page);
		if (find_ret != data->pages.end())
		{
			return find_ret->second;
		}

		throw ENGINE_EXCEPTION("Failed getting page texture!");
	}

	std::shared_ptr<const Texture> Font::getPageTexture(int page) const
	{
		auto find_ret = data->pages.find(page);
		if (find_ret != data->pages.end())
		{
			return find_ret->second;
		}

		throw ENGINE_EXCEPTION("Failed getting page texture!");
	}

	std::optional<const FontChar*> Font::getChar(char character) const
	{
		auto find_ret = data->chars.find(character);
		if (find_ret != data->chars.end())
		{
			return &find_ret->second;
		}
		return {};
	}

	std::optional<std::string> Font::getFontInfoParameter(const std::string& name) const
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
