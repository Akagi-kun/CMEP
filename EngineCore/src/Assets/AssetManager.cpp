#include "Assets/AssetManager.hpp"

#include "Assets/Asset.hpp"
#include "Assets/Font.hpp"

#include "Scripting/ILuaScript.hpp"

#include "Logging/Logging.hpp"

#include "Exception.hpp"
#include "InternalEngineObject.hpp"

#include <cassert>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>

namespace Engine
{
	struct AssetRepository
	{
		AssetRepository() = default;
		~AssetRepository()
		{
			fonts.clear();

			for (const auto& texture : textures)
			{
				if (texture.second.use_count() > 1)
				{
					assert(false && "Likely leaked a texture!");
				}
			}

			textures.clear();
		}

		struct
		{
			// Offset initial values (by 1/3 of the maximal number) so that they don't match up
			Asset::uid_t script	 = 0;
			Asset::uid_t font	 = std::numeric_limits<Asset::uid_t>::max() / 3;
			Asset::uid_t texture = std::numeric_limits<Asset::uid_t>::max() / 3 * 2;
		} last_uid;

		/**
		 * @brief A container that holds assets of a specific type
		 *
		 * @tparam val_t The type of asset
		 */
		template <typename val_t>
			requires IsAsset<val_t>
		using map_t = std::unordered_map<std::string, std::shared_ptr<val_t>>;

		map_t<Scripting::ILuaScript> luascripts;
		map_t<Rendering::Texture>	 textures;
		map_t<Rendering::Font>		 fonts;

		template <AssetType type> auto getAsset(const std::string& name)
		{
			if constexpr (type == AssetType::FONT)
			{
				auto found = fonts.find(name);
				if (found != fonts.end()) { return found->second; }
			}
			else if constexpr (type == AssetType::SCRIPT)
			{
				auto found = luascripts.find(name);
				if (found != luascripts.end()) { return found->second; }
			}
			else if constexpr (type == AssetType::TEXTURE)
			{
				auto found = textures.find(name);
				if (found != textures.end()) { return found->second; }
			}

			// When no match found
			// Return nullptr cast to the matching return type
			return decltype(getAsset<type>(name))(nullptr);
		}
	};

#pragma region Public

	AssetManager::AssetManager(Engine* with_engine) : InternalEngineObject(with_engine)
	{
		repository = new AssetRepository();
	}

	AssetManager::~AssetManager()
	{
		this->logger->simpleLog<decltype(this)>(
			Logging::LogLevel::Info,
			"Destructor called"
		);

		cleanRepository();

		delete repository;
	}

	void AssetManager::addTexture(
		const std::string&						   name,
		const std::shared_ptr<Rendering::Texture>& asset
	)
	{
		asset->assignUID(repository->last_uid.texture++);
		repository->textures.emplace(name, asset);
	}

	void AssetManager::addFont(
		const std::string&						name,
		const std::shared_ptr<Rendering::Font>& asset
	)
	{
		asset->assignUID(repository->last_uid.font++);
		repository->fonts.emplace(name, asset);
	}

	void AssetManager::addLuaScript(
		const std::string&							  name,
		const std::shared_ptr<Scripting::ILuaScript>& asset
	)
	{
		asset->assignUID(repository->last_uid.script++);
		repository->luascripts.emplace(name, asset);
	}

	std::shared_ptr<Rendering::Texture> AssetManager::getTexture(const std::string& name)
	{
		return repository->getAsset<AssetType::TEXTURE>(name);
	}

	std::shared_ptr<Rendering::Font> AssetManager::getFont(const std::string& name)
	{
		return repository->getAsset<AssetType::FONT>(name);
	}

	std::shared_ptr<Scripting::ILuaScript>
	AssetManager::getLuaScript(const std::string& name)
	{
		return repository->getAsset<AssetType::SCRIPT>(name);
	}

	[[nodiscard]] std::shared_ptr<void>
	AssetManager::getAsset(AssetType with_type, const std::string& name)
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

	void AssetManager::cleanRepository()
	{
		this->logger->simpleLog<decltype(this)>(
			Logging::LogLevel::Debug,
			"Cleaned Asset repository"
		);

		repository->luascripts.clear();

		repository->fonts.clear();

		for (auto& texture : repository->textures)
		{
			this->logger->simpleLog<decltype(this)>(
				Logging::LogLevel::VerboseDebug,
				"Texture '%s' use_count: %u",
				texture.first.c_str(),
				texture.second.use_count()
			);
			texture.second.reset();
		}

		repository->textures.clear();
	}

#pragma endregion

} // namespace Engine
