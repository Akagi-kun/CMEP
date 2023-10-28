#include "Rendering/Texture.hpp"
#include "Rendering/Shader.hpp"
#include "Logging/Logging.hpp"
#include "Rendering/Font.hpp"
#include "AssetManager.hpp"

namespace Engine
{
#pragma region Adding Assets
	void AssetManager::AddShader(std::string name, std::string vert_source, std::string frag_source)
	{
		this->shaders.emplace(name, new Engine::Rendering::Shader(vert_source.c_str(), frag_source.c_str()));
		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2, "Added shader %s", name.c_str());
	}

	void AssetManager::AddTexture(std::string name, std::string path, Rendering::Texture_InitFiletype filetype)
	{
		Rendering::Texture* texture = new Engine::Rendering::Texture();

		if (texture->InitFile(filetype, path) != 0)
		{
			delete texture;
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "Error occured when adding Texture %s as %s, this may be unintentional", path.c_str(), name.c_str());
			return;
		}

		this->textures.emplace(name, texture);
		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2, "Added texture %s as %s", path.c_str(), name.c_str());
	}

	void AssetManager::AddFont(std::string name, std::string path)
	{
		Rendering::Font* font = new Engine::Rendering::Font(this);
		font->Init(path);

		this->fonts.emplace(name, font);
	}

	void AssetManager::AddLuaScript(std::string name, std::string path)
	{
		Scripting::LuaScript* script = new Scripting::LuaScript(path);

		this->luascripts.emplace(name, script);
	}
#pragma endregion

#pragma region Getting Assets
	Rendering::Texture* AssetManager::GetTexture(std::string name)
	{
		if (this->textures.find(name) != this->textures.end())
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2, "Texture %s requested and is loaded", name.c_str());
			return this->textures.at(name);
		}
		else
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Warning, "Texture %s requested and is not loaded", name.c_str());
			return nullptr;
			/*
			Rendering::Texture* texture = new Engine::Rendering::Texture();

			if (texture->InitFile(Rendering::Texture_InitFiletype::FILE_NETPBM, path) != 0)
			{
				delete texture;
				return nullptr;
			}

			this->textures.emplace(path, texture);
			return texture;
			*/
		}
	}
	

	Rendering::Shader* AssetManager::GetShader(std::string name)
	{
		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2, "Shader %s requested", name.c_str());
		if (this->shaders.find(name) != this->shaders.end())
		{
			return this->shaders.at(name);
		}
		else
		{
			return nullptr;
		}
	}

	Rendering::Font* AssetManager::GetFont(std::string name)
	{
		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2, "Font %s requested", name.c_str());
		if (this->fonts.find(name) != this->fonts.end())
		{
			return this->fonts.at(name);
		}
		else
		{
			Rendering::Font* font = new Engine::Rendering::Font(this);
			
			if (font->Init(name) != 0)
			{
				delete font;
				return nullptr;
			}
			
			this->fonts.emplace(name, font);
			return font;
		}
	}

	Scripting::LuaScript* AssetManager::GetLuaScript(std::string name)
	{
		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2, "LuaScript %s requested", name.c_str());
		if (this->luascripts.find(name) != this->luascripts.end())
		{
			return this->luascripts.at(name);
		}
		else
		{
			return nullptr;
		}
	}
#pragma endregion
}