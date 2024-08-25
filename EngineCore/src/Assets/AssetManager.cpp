#include "Assets/AssetManager.hpp"

#include "Assets/Font.hpp"

#include "Scripting/ILuaScript.hpp"

#include "Logging/Logging.hpp"

#include "InternalEngineObject.hpp"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_ASSET_MANAGER
#include "Logging/LoggingPrefix.hpp" // IWYU pragma: keep

namespace Engine
{
	AssetManager::AssetManager(Engine* with_engine) : InternalEngineObject(with_engine)
	{
		font_factory	= std::make_unique<Factories::FontFactory>(with_engine);
		texture_factory = std::make_unique<Factories::TextureFactory>(with_engine);
	}

	AssetManager::~AssetManager()
	{
		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Destructor called");

		fonts.clear();

		for (auto& texture : textures)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Debug3,
				LOGPFX_CURRENT "Texture '%s' use_count: %u",
				texture.first.c_str(),
				texture.second.use_count()
			);
			texture.second.reset();
		}

		textures.clear();
	}

#pragma region Adding Assets
	void AssetManager::AddTexture(
		const std::string& name,
		const std::filesystem::path& path,
		Rendering::Texture_InitFiletype filetype,
		vk::Filter filtering,
		vk::SamplerAddressMode address_mode
	)
	{
		std::shared_ptr<Rendering::Texture> texture =
			texture_factory->InitFile(path, filetype, filtering, address_mode);

		texture->AssignUID(last_uid.texture++);

		textures.emplace(name, texture);
		this->logger->SimpleLog(
			Logging::LogLevel::Debug2,
			LOGPFX_CURRENT "Added texture '%s' as '%s'",
			path.c_str(),
			name.c_str()
		);
	}

	void AssetManager::AddFont(const std::string& name, const std::filesystem::path& path)
	{
		std::shared_ptr<Rendering::Font> font = font_factory->InitBMFont(path);
		font->AssignUID(last_uid.font++);

		fonts.emplace(name, std::move(font));
	}

	void AssetManager::AddLuaScript(const std::string& name, const std::shared_ptr<Scripting::ILuaScript>& script)
	{
		script->AssignUID(last_uid.script++);
		luascripts.emplace(name, script);
	}

#pragma endregion

#pragma region Getting Assets
	std::shared_ptr<Rendering::Texture> AssetManager::GetTexture(const std::string& name)
	{
		if (textures.find(name) != textures.end())
		{
			return textures.at(name);
		}

		this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Texture asset '%s' not found", name.c_str());

		return nullptr;
	}

	std::shared_ptr<Rendering::Font> AssetManager::GetFont(const std::string& name)
	{
		if (fonts.find(name) != fonts.end())
		{
			return fonts.at(name);
		}

		this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Font asset '%s' not found", name.c_str());

		return nullptr;
	}

	std::shared_ptr<Scripting::ILuaScript> AssetManager::GetLuaScript(const std::string& name)
	{
		if (luascripts.find(name) != luascripts.end())
		{
			return luascripts.at(name);
		}

		this->logger
			->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "LuaScript asset '%s' not found", name.c_str());

		return nullptr;
	}
#pragma endregion
} // namespace Engine
