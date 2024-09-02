#pragma once

#include "Assets/Texture.hpp"

#include "Scripting/ILuaScript.hpp"

#include "InternalEngineObject.hpp"

#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>

namespace Engine
{
	namespace Rendering
	{
		class Font;
	}

	enum class AssetType : uint8_t
	{
		FONT	= 4,
		TEXTURE = 20,
		SCRIPT	= 12
	};

	class AssetManager final : public InternalEngineObject
	{
	public:
		AssetManager(Engine* with_engine);
		~AssetManager();

		void AddTexture(const std::string& name, const std::shared_ptr<Rendering::Texture>& asset);

		void AddFont(const std::string& name, const std::shared_ptr<Rendering::Font>& asset);

		void AddLuaScript(
			const std::string& name,
			const std::shared_ptr<Scripting::ILuaScript>& asset
		);

		[[nodiscard]] std::shared_ptr<Rendering::Texture> GetTexture(const std::string& name);
		[[nodiscard]] std::shared_ptr<Rendering::Font> GetFont(const std::string& name);
		[[nodiscard]] std::shared_ptr<Scripting::ILuaScript> GetLuaScript(const std::string& name);

		[[deprecated, nodiscard]] std::shared_ptr<void> GetAsset(
			AssetType with_type,
			const std::string& name
		);

	private:
		struct
		{
			// Offset initial values so that they don't match up
			uint_least32_t script  = 0;
			uint_least32_t font	   = std::numeric_limits<uint_least32_t>::max() / 3;
			uint_least32_t texture = std::numeric_limits<uint_least32_t>::max() / 3 * 2;
		} last_uid;

		std::unordered_map<std::string, std::shared_ptr<Scripting::ILuaScript>> luascripts;
		std::unordered_map<std::string, std::shared_ptr<Rendering::Texture>> textures;
		std::unordered_map<std::string, std::shared_ptr<Rendering::Font>> fonts;
	};
} // namespace Engine
