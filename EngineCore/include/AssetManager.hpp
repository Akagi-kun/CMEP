#pragma once

#include <unordered_map>
#include <string>

#include "Scripting/LuaScript.hpp"
#include "PlatformSemantics.hpp"

namespace Engine
{
	namespace Rendering
	{
		typedef enum class Texture_InitFiletypeEnum Texture_InitFiletype;
		class Texture;
		class Shader;
		class Font;
	}

	class CMEP_EXPORT AssetManager final
	{
	private:
		std::unordered_map<std::string, Scripting::LuaScript*> luascripts;
		std::unordered_map<std::string, Rendering::Texture*> textures;
		std::unordered_map<std::string, Rendering::Shader*> shaders;
		std::unordered_map<std::string, Rendering::Font*> fonts;
	public:
		AssetManager() {};

		void AddShader(std::string name, std::string vert_source, std::string frag_source);
		void AddTexture(std::string name, std::string path, Rendering::Texture_InitFiletype filetype);
		void AddFont(std::string name, std::string path);
		void AddLuaScript(std::string name, std::string path);

		Rendering::Shader* GetShader(std::string name);
		Rendering::Texture* GetTexture(std::string name);
		Rendering::Font* GetFont(std::string name);
		Scripting::LuaScript* GetLuaScript(std::string name);
	};
}