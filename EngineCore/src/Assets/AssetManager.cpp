#include "Assets/AssetManager.hpp"

#include "Assets/Font.hpp"

#include "Scripting/ILuaScript.hpp"

#include "Logging/Logging.hpp"

#include "Exception.hpp"
#include "InternalEngineObject.hpp"

#include <memory>
#include <string>

namespace Engine
{

#pragma region Public

	AssetManager::AssetManager(Engine* with_engine) : InternalEngineObject(with_engine)
	{
	}

	AssetManager::~AssetManager()
	{
		this->logger->simpleLog<decltype(this)>(Logging::LogLevel::Info, "Destructor called");

		fonts.clear();

		for (auto& texture : textures)
		{
			this->logger->simpleLog<decltype(this)>(
				Logging::LogLevel::VerboseDebug,
				"Texture '%s' use_count: %u",
				texture.first.c_str(),
				texture.second.use_count()
			);
			texture.second.reset();
		}

		textures.clear();
	}

#pragma region Adding Assets

	void AssetManager::addTexture(
		const std::string&						   name,
		const std::shared_ptr<Rendering::Texture>& asset
	)
	{
		asset->assignUID(last_uid.texture++);
		textures.emplace(name, asset);
	}

	void AssetManager::addFont(const std::string& name, const std::shared_ptr<Rendering::Font>& asset)
	{
		asset->assignUID(last_uid.font++);
		fonts.emplace(name, asset);
	}

	void AssetManager::addLuaScript(
		const std::string&							  name,
		const std::shared_ptr<Scripting::ILuaScript>& asset
	)
	{
		asset->assignUID(last_uid.script++);
		luascripts.emplace(name, asset);
	}

#pragma endregion

#pragma region Getting Assets

	std::shared_ptr<Rendering::Texture> AssetManager::getTexture(const std::string& name)
	{
		if (textures.find(name) != textures.end())
		{
			return textures.at(name);
		}

		this->logger->simpleLog<decltype(this)>(
			Logging::LogLevel::VerboseDebug,
			"Texture asset '%s' not found",
			name.c_str()
		);

		return nullptr;
	}

	std::shared_ptr<Rendering::Font> AssetManager::getFont(const std::string& name)
	{
		if (fonts.find(name) != fonts.end())
		{
			return fonts.at(name);
		}

		this->logger->simpleLog<decltype(this)>(
			Logging::LogLevel::VerboseDebug,
			"Font asset '%s' not found",
			name.c_str()
		);

		return nullptr;
	}

	std::shared_ptr<Scripting::ILuaScript> AssetManager::getLuaScript(const std::string& name)
	{
		if (luascripts.find(name) != luascripts.end())
		{
			return luascripts.at(name);
		}

		this->logger->simpleLog<decltype(this)>(
			Logging::LogLevel::VerboseDebug,
			"LuaScript asset '%s' not found",
			name.c_str()
		);

		return nullptr;
	}

	[[nodiscard]] std::shared_ptr<void> AssetManager::getAsset(
		AssetType		   with_type,
		const std::string& name
	)
	{
		switch (with_type)
		{
			case AssetType::FONT:
			{
				return getFont(name);
			}
			case AssetType::SCRIPT:
			{
				return getLuaScript(name);
			}
			case AssetType::TEXTURE:
			{
				return getTexture(name);
			}
			default:
			{
				throw ENGINE_EXCEPTION("Invalid asset type!");
			}
		}
	}

#pragma endregion

#pragma endregion

} // namespace Engine
