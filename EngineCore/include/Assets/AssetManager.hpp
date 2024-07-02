#pragma once

#include "Assets/Mesh.hpp"
#include "Assets/Texture.hpp"

#include "Scripting/LuaScript.hpp"

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
		std::unordered_map<std::string, std::shared_ptr<Scripting::LuaScript>> luascripts;
		std::unordered_map<std::string, std::shared_ptr<Rendering::Texture>> textures;
		std::unordered_map<std::string, std::shared_ptr<Rendering::Font>> fonts;
		std::unordered_map<std::string, std::shared_ptr<Rendering::Mesh>> models;

		std::unique_ptr<Factories::FontFactory> font_factory;
		std::unique_ptr<Factories::TextureFactory> texture_factory;

	public:
		Scripting::LuaScriptExecutor* lua_executor{};

		AssetManager(Engine* with_engine);
		~AssetManager();

		void AddTexture(
			std::string name,
			const std::string& path,
			Rendering::Texture_InitFiletype filetype,
			VkFilter filtering				  = VK_FILTER_LINEAR,
			VkSamplerAddressMode address_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT
		);
		void AddFont(const std::string& name, const std::string& path);
		void AddLuaScript(const std::string& name, const std::string& path);
		void AddModel(const std::string& name, const std::string& path);

		std::shared_ptr<Rendering::Texture> GetTexture(const std::string& name);
		std::shared_ptr<Rendering::Font> GetFont(const std::string& name);
		std::shared_ptr<Scripting::LuaScript> GetLuaScript(const std::string& name);
		std::shared_ptr<Rendering::Mesh> GetModel(const std::string& name);
	};
} // namespace Engine
