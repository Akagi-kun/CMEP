#include "Rendering/Texture.hpp"
#include "Logging/Logging.hpp"
#include "Rendering/Font.hpp"
#include "AssetManager.hpp"

namespace Engine
{
	AssetManager::~AssetManager()
	{
		for(auto& texture : this->textures)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug3, "Texture %s use_count: %u", texture.first.c_str(), texture.second.use_count());
		}

		this->textures.clear();

		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug3, "Deleting asset manager");
	}

#pragma region Adding Assets
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
		
		font->Init(std::move(path));

		this->fonts.emplace(name, font);
	}

	void AssetManager::AddLuaScript(std::string name, std::string path)
	{
		Scripting::LuaScript* script = new Scripting::LuaScript(std::move(path));

		this->luascripts.emplace(name, script);
	}
#pragma endregion

#pragma region Getting Assets
	std::shared_ptr<Rendering::Texture> AssetManager::GetTexture(std::string name)
	{
		if (this->textures.find(name) != this->textures.end())
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2, "Texture %s requested and is loaded", name.c_str());
			return this->textures.at(name);
		}
		else
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Warning, "Texture %s requested and is not loaded", name.c_str());
			this->AddTexture(name, name, Rendering::Texture_InitFiletype::FILE_PNG);
			
			return this->GetTexture(name);
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

	std::shared_ptr<Rendering::Font> AssetManager::GetFont(std::string name)
	{
		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2, "Font %s requested", name.c_str());
		if (this->fonts.find(name) != this->fonts.end())
		{
			return this->fonts.at(name);
		}
		else
		{
			std::shared_ptr<Rendering::Font> font = std::make_shared<Rendering::Font>(this);
			
			if (font->Init(name) != 0)
			{
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