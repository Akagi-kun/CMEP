#include "Assets/AssetManager.hpp"

#include "Assets/Font.hpp"

#include "Scripting/ILuaScript.hpp"

#include "Logging/Logging.hpp"

#include "Exception.hpp"
#include "InternalEngineObject.hpp"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_ASSET_MANAGER
#include "Logging/LoggingPrefix.hpp"

namespace Engine
{
	AssetManager::AssetManager(Engine* with_engine) : InternalEngineObject(with_engine)
	{
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
		const std::shared_ptr<Rendering::Texture>& asset
	)
	{
		asset->AssignUID(last_uid.texture++);
		textures.emplace(name, asset);
	}

	void AssetManager::AddFont(
		const std::string& name,
		const std::shared_ptr<Rendering::Font>& asset
	)
	{
		asset->AssignUID(last_uid.font++);

		fonts.emplace(name, asset);
	}

	void AssetManager::AddLuaScript(
		const std::string& name,
		const std::shared_ptr<Scripting::ILuaScript>& asset
	)
	{
		asset->AssignUID(last_uid.script++);
		luascripts.emplace(name, asset);
	}

#pragma endregion

#pragma region Getting Assets
	std::shared_ptr<Rendering::Texture> AssetManager::GetTexture(const std::string& name)
	{
		if (textures.find(name) != textures.end())
		{
			return textures.at(name);
		}

		this->logger->SimpleLog(
			Logging::LogLevel::Debug2,
			LOGPFX_CURRENT "Texture asset '%s' not found",
			name.c_str()
		);

		return nullptr;
	}

	std::shared_ptr<Rendering::Font> AssetManager::GetFont(const std::string& name)
	{
		if (fonts.find(name) != fonts.end())
		{
			return fonts.at(name);
		}

		this->logger->SimpleLog(
			Logging::LogLevel::Debug2,
			LOGPFX_CURRENT "Font asset '%s' not found",
			name.c_str()
		);

		return nullptr;
	}

	std::shared_ptr<Scripting::ILuaScript> AssetManager::GetLuaScript(const std::string& name)
	{
		if (luascripts.find(name) != luascripts.end())
		{
			return luascripts.at(name);
		}

		this->logger->SimpleLog(
			Logging::LogLevel::Debug2,
			LOGPFX_CURRENT "LuaScript asset '%s' not found",
			name.c_str()
		);

		return nullptr;
	}

	[[nodiscard]] std::shared_ptr<void> AssetManager::GetAsset(
		AssetType with_type,
		const std::string& name
	)
	{
		switch (with_type)
		{
			case AssetType::FONT:
			{
				return GetFont(name);
			}
			case AssetType::SCRIPT:
			{
				return GetLuaScript(name);
			}
			case AssetType::TEXTURE:
			{
				return GetTexture(name);
			}
			default:
			{
				throw ENGINE_EXCEPTION("Invalid asset type!");
			}
		}
	}
#pragma endregion
} // namespace Engine
