#pragma once

#include "Assets/Asset.hpp"
#include "Assets/Font.hpp"
#include "Assets/Texture.hpp"

#include "Scripting/ILuaScript.hpp"

#include "Logging/Logging.hpp"

#include "Exception.hpp"

#include <cassert>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace Engine
{
	enum class AssetType : uint8_t
	{
		FONT	= 4,
		TEXTURE = 8,
		SCRIPT	= 12
	};

	/**
	 * Common point to store and maintain Assets
	 */
	class AssetRepository
	{
	public:
		/**
		 * Container that holds a single asset entry
		 * @tparam asset_t Type of asset
		 */
		template <typename asset_t>
			requires IsAsset<asset_t>
		using asset_entry_t = std::shared_ptr<asset_t>;

		AssetRepository() = default;
		~AssetRepository()
		{
			clear();
		}

		AssetRepository(AssetRepository&)			 = delete;
		AssetRepository& operator=(AssetRepository&) = delete;

		/**
		 * Get an asset from this repository
		 *
		 * @tparam asset_t Type of asset
		 *
		 * @param name Name to search for
		 *
		 * @return Asset or @code nullptr @endcode if none found
		 */
		template <typename asset_t>
		[[nodiscard]] auto getAsset(const std::string& name) -> std::optional<asset_entry_t<asset_t>>
			requires(IsAsset<asset_t>)
		{
			auto map = getAssetMap<asset_t>();

			auto found = map.find(name);
			if (found != map.end()) { return found->second; }

			// When no match found
			// Return nullptr cast to the matching return type
			return {};
		}

		/**
		 * Add an asset to this repository
		 *
		 * @tparam asset_t Type of asset (can be deduced from @p asset )
		 *
		 * @param name Name for this asset, must be unique for assets sharing a type
		 *              (i.e. you can have 1 texture and 1 font asset of same name, but not 2 textures)
		 * @param asset Pointer to the asset
		 */
		template <typename asset_t>
		void addAsset(const std::string& name, const std::shared_ptr<asset_t>& asset)
			requires(IsAsset<asset_t>)
		{
			assert((asset) && "Passed invalid asset");
			assert((!name.empty()) && "Empty asset names are invalid");

			asset->assignUID(getNextUID<asset_t>());

			auto& map = getAssetMap<asset_t>();

			EXCEPTION_ASSERT(!map.contains(name), "Asset already exists!");

			map.emplace(name, asset);
		}

		/**
		 * Removes all assets from this repository
		 */
		void clear()
		{
			fonts.clear();
			textures.clear();
			luascripts.clear();
		}

	protected:
		/**
		 * Container that maps names to asset entries
		 * @tparam asset_t Type of asset
		 */
		template <typename asset_t>
			requires IsAsset<asset_t>
		using map_t = std::unordered_map<std::string, asset_entry_t<asset_t>>;

		map_t<Scripting::ILuaScript> luascripts;
		map_t<Rendering::Texture>	 textures;
		map_t<Rendering::Font>		 fonts;

		struct
		{
			Asset::uid_t script	 = Asset::getUIDSubdivide(0, 3);
			Asset::uid_t font	 = Asset::getUIDSubdivide(1, 3);
			Asset::uid_t texture = Asset::getUIDSubdivide(2, 3);
		} last_uid;

		/**
		 * Utility to get the map that holds assets of specified type
		 *
		 * @tparam asset_t Type of asset
		 *
		 * @return Reference to the map
		 */
		template <typename asset_t>
		auto& getAssetMap()
			requires(IsAsset<asset_t>)
		{
			if constexpr (std::is_same_v<asset_t, Rendering::Font>) { return fonts; }
			else if constexpr (std::is_same_v<asset_t, Rendering::Texture>) { return textures; }
			else if constexpr (std::is_same_v<asset_t, Scripting::ILuaScript>)
			{
				return luascripts;
			}
		}

		template <typename asset_t> Asset::uid_t getNextUID()
		{
			if constexpr (std::is_same_v<asset_t, Rendering::Font>) { return last_uid.font++; }
			else if constexpr (std::is_same_v<asset_t, Rendering::Texture>)
			{
				return last_uid.texture++;
			}
			else if constexpr (std::is_same_v<asset_t, Scripting::ILuaScript>)
			{
				return last_uid.script++;
			}
		}
	};
	static_assert(!std::is_copy_constructible_v<AssetRepository> && !std::is_copy_assignable_v<AssetRepository>);

	class AssetManager final : public Logging::SupportsLogging
	{
	public:
		AssetManager(const Logging::SupportsLogging::logger_t& with_logger);
		~AssetManager();

		/**
		 * @sa AssetRepository::getAsset()
		 */
		template <typename asset_t>
		auto getAsset(const std::string& name)
			requires(IsAsset<asset_t>)
		{
			assert(repository);
			return repository->getAsset<asset_t>(name);
		}

		void setSceneRepository(AssetRepository* with_repository);

		/**
		 * @sa AssetRepository::clear()
		 */
		void clearRepository()
		{
			assert(repository);
			repository->clear();
		}

	private:
		AssetRepository* repository = nullptr;
	};
} // namespace Engine
