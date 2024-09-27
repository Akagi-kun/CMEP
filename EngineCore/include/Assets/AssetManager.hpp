#pragma once

#include "Assets/Texture.hpp"

#include "Scripting/ILuaScript.hpp"

#include "InternalEngineObject.hpp"

#include <cstdint>
#include <memory>
#include <string>

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

	struct AssetRepository;

	class AssetManager final : public InternalEngineObject
	{
	public:
		AssetManager(Engine* with_engine);
		~AssetManager();

		void
		addTexture(const std::string& name, const std::shared_ptr<Rendering::Texture>& asset);

		void
		addFont(const std::string& name, const std::shared_ptr<Rendering::Font>& asset);

		void addLuaScript(
			const std::string&							  name,
			const std::shared_ptr<Scripting::ILuaScript>& asset
		);

		[[nodiscard]] std::shared_ptr<Rendering::Texture>
		getTexture(const std::string& name);
		[[nodiscard]] std::shared_ptr<Rendering::Font> getFont(const std::string& name);
		[[nodiscard]] std::shared_ptr<Scripting::ILuaScript>
		getLuaScript(const std::string& name);

		[[deprecated, nodiscard]] std::shared_ptr<void>
		getAsset(AssetType with_type, const std::string& name);

	private:
		AssetRepository* repository;
	};
} // namespace Engine
