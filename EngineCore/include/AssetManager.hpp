#pragma once

#include <unordered_map>
#include <string>

#include "Scripting/LuaScript.hpp"
#include "PlatformSemantics.hpp"
#include "Rendering/Texture.hpp"

namespace Engine
{
	namespace Rendering
	{
		class Font;
	}

	class CMEP_EXPORT AssetManager final
	{
	private:
		std::unordered_map<std::string, Scripting::LuaScript*> luascripts;
		std::unordered_map<std::string, std::shared_ptr<Rendering::Texture>> textures;
		std::unordered_map<std::string, Rendering::Font*> fonts;
	public:
		AssetManager() {};

		void AddTexture(std::string name, std::string path, Rendering::Texture_InitFiletype filetype);
		void AddFont(std::string name, std::string path);
		void AddLuaScript(std::string name, std::string path);

		std::shared_ptr<Rendering::Texture> GetTexture(std::string name);
		Rendering::Font* GetFont(std::string name);
		Scripting::LuaScript* GetLuaScript(std::string name);
	};
}