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

		void addTexture(const std::string& name, const std::shared_ptr<Rendering::Texture>& asset);

		void addFont(const std::string& name, const std::shared_ptr<Rendering::Font>& asset);

		void addLuaScript(
			const std::string&							  name,
			const std::shared_ptr<Scripting::ILuaScript>& asset
		);

		[[nodiscard]] std::shared_ptr<Rendering::Texture> getTexture(const std::string& name);
		[[nodiscard]] std::shared_ptr<Rendering::Font> getFont(const std::string& name);
		[[nodiscard]] std::shared_ptr<Scripting::ILuaScript> getLuaScript(const std::string& name
		);

		[[deprecated, nodiscard]] std::shared_ptr<void> getAsset(
			AssetType		   with_type,
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
		std::unordered_map<std::string, std::shared_ptr<Rendering::Font>>	 fonts;
	};
} // namespace Engine
