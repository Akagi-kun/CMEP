#pragma once

#include "Assets/Texture.hpp"

#include "Scripting/ILuaScript.hpp"

#include "Factories/FontFactory.hpp"
#include "Factories/TextureFactory.hpp"

#include "InternalEngineObject.hpp"

#include <string>
#include <unordered_map>

namespace Engine
{
	namespace Rendering
	{
		class Font;
	}

	class AssetManager final : public InternalEngineObject
	{
	private:
		std::unordered_map<std::string, std::shared_ptr<Scripting::ILuaScript>> luascripts;
		std::unordered_map<std::string, std::shared_ptr<Rendering::Texture>> textures;
		std::unordered_map<std::string, std::shared_ptr<Rendering::Font>> fonts;

		std::unique_ptr<Factories::FontFactory> font_factory;
		std::unique_ptr<Factories::TextureFactory> texture_factory;

	public:
		Scripting::LuaScriptExecutor* lua_executor{};

		AssetManager(Engine* with_engine);
		~AssetManager();

		void AddTexture(
			const std::string& name,
			const std::string& path,
			Rendering::Texture_InitFiletype filetype,
			vk::Filter filtering				= vk::Filter::eLinear,
			vk::SamplerAddressMode address_mode = vk::SamplerAddressMode::eRepeat
		);
		void AddFont(const std::string& name, const std::string& path);

		void AddLuaScript(const std::string& name, const std::shared_ptr<Scripting::ILuaScript>& script);

		std::shared_ptr<Rendering::Texture> GetTexture(const std::string& name);
		std::shared_ptr<Rendering::Font> GetFont(const std::string& name);
		std::shared_ptr<Scripting::ILuaScript> GetLuaScript(const std::string& name);
	};
} // namespace Engine
