#pragma once

#include <unordered_map>
#include <string>

#include "Scripting/LuaScript.hpp"
#include "PlatformSemantics.hpp"
#include "Rendering/Texture.hpp"
#include "Rendering/Mesh.hpp"
#include "InternalEngineObject.hpp"

namespace Engine
{
	namespace Rendering
	{
		class Font;
	}

	class CMEP_EXPORT AssetManager final
	{
	private:
		std::unordered_map<std::string, std::shared_ptr<Scripting::LuaScript>> luascripts;
		std::unordered_map<std::string, std::shared_ptr<Rendering::Texture>> textures;
		std::unordered_map<std::string, std::shared_ptr<Rendering::Font>> fonts;
		std::unordered_map<std::string, std::shared_ptr<Rendering::Mesh>> models;
	public:
		std::shared_ptr<Logging::Logger> logger;
		Scripting::LuaScriptExecutor* lua_executor;

		AssetManager() {};
		~AssetManager();

		void AddTexture(std::string name, std::string path, Rendering::Texture_InitFiletype filetype);
		void AddFont(std::string name, std::string path);
		void AddLuaScript(std::string name, std::string path);
		void AddModel(std::string name, std::string path);

		std::shared_ptr<Rendering::Texture> GetTexture(std::string name);
		std::shared_ptr<Rendering::Font> GetFont(std::string name);
		std::shared_ptr<Scripting::LuaScript> GetLuaScript(std::string name);
		std::shared_ptr<Rendering::Mesh> GetModel(std::string name);
	};
}