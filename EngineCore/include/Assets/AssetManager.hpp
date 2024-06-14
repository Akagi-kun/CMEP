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
		// std::shared_ptr<Logging::Logger> logger{};
		// Engine* owner_engine{};

		Scripting::LuaScriptExecutor* lua_executor{};

		// std::string current_load_path = "";

		AssetManager();
		~AssetManager();

		// Overrides InternalEngineObject::UpdateHeldLogger
		void UpdateHeldLogger(std::shared_ptr<Logging::Logger> new_logger);
		// Overrides InternalEngineObject::UpdateOwnerEngine
		void UpdateOwnerEngine(Engine* new_owner_engine);

		void AddTexture(
			std::string name,
			std::string path,
			Rendering::Texture_InitFiletype filetype,
			VkFilter filtering = VK_FILTER_LINEAR,
			VkSamplerAddressMode address_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT
		);
		void AddFont(std::string name, std::string path);
		void AddLuaScript(std::string name, std::string path);
		void AddModel(std::string name, std::string path);

		std::shared_ptr<Rendering::Texture> GetTexture(std::string name);
		std::shared_ptr<Rendering::Font> GetFont(std::string name);
		std::shared_ptr<Scripting::LuaScript> GetLuaScript(std::string name);
		std::shared_ptr<Rendering::Mesh> GetModel(std::string name);
	};
} // namespace Engine
