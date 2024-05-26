#include "Logging/Logging.hpp"
#include "Rendering/Font.hpp"
#include "AssetManager.hpp"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_ASSET_MANAGER
#include "Logging/LoggingPrefix.hpp"

namespace Engine
{
	AssetManager::AssetManager()
	{
		this->fontFactory = std::make_unique<Factories::FontFactory>(this);
	}

	AssetManager::~AssetManager()
	{
		for(auto& texture : this->textures)
		{
			this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Texture %s use_count: %u", texture.first.c_str(), texture.second.use_count());
		}

		this->textures.clear();

		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Deleting asset manager");
	}

	void AssetManager::UpdateHeldLogger(std::shared_ptr<Logging::Logger> new_logger)
	{
		InternalEngineObject::UpdateHeldLogger(new_logger);
		this->fontFactory->UpdateHeldLogger(new_logger);
	}

    void AssetManager::UpdateOwnerEngine(Engine* new_owner_engine)
	{
		InternalEngineObject::UpdateOwnerEngine(new_owner_engine);
		this->fontFactory->UpdateOwnerEngine(new_owner_engine);
	}

#pragma region Adding Assets
	void AssetManager::AddTexture(std::string name, std::string path, Rendering::Texture_InitFiletype filetype)
	{
		std::shared_ptr<Rendering::Texture> texture = std::make_shared<Rendering::Texture>();
		texture->UpdateOwnerEngine(this->owner_engine);

		texture->UpdateHeldLogger(this->logger);

		if (texture->InitFile(filetype, this->current_load_path + path) != 0)
		{
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Error occured when adding Texture %s as %s, this may be unintentional", path.c_str(), name.c_str());
			return;
		}

		this->textures.emplace(name, texture);
		this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Added texture %s as %s", path.c_str(), name.c_str());
	}

	void AssetManager::AddFont(std::string name, std::string path)
	{
		std::shared_ptr<Rendering::Font> font = this->fontFactory->InitBMFont(path);
		this->fonts.emplace(name, std::move(font));
	}

	void AssetManager::AddLuaScript(std::string name, std::string path)
	{
		Scripting::LuaScript* script = new Scripting::LuaScript(this->lua_executor, std::move(path));
		
		this->luascripts.emplace(name, script);
	}

	void AssetManager::AddModel(std::string name, std::string path)
	{
		std::shared_ptr<Rendering::Mesh> mesh = std::make_shared<Rendering::Mesh>();
		mesh->UpdateOwnerEngine(this->owner_engine);
		mesh->UpdateHeldLogger(this->logger);
		
		mesh->CreateMeshFromObj(this->current_load_path + path);

		this->models.emplace(name, std::move(mesh));
	}
#pragma endregion

#pragma region Getting Assets
	std::shared_ptr<Rendering::Texture> AssetManager::GetTexture(std::string name)
	{
		if (this->textures.find(name) != this->textures.end())
		{
			this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Texture %s requested and is loaded", name.c_str());
			return this->textures.at(name);
		}
		else
		{
			this->logger->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "Texture %s requested and is not loaded", name.c_str());
			this->AddTexture(name, name, Rendering::Texture_InitFiletype::FILE_PNG);
			
			return this->GetTexture(name);
		}
	}

	std::shared_ptr<Rendering::Font> AssetManager::GetFont(std::string name)
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Font %s requested", name.c_str());
		if (this->fonts.find(name) != this->fonts.end())
		{
			return this->fonts.at(name);
		}
		else
		{
			// Recursive!!!
			this->AddFont(name, this->current_load_path + name);
			return this->GetFont(name);

			//std::shared_ptr<Rendering::Font> font = std::make_shared<Rendering::Font>(this);
			//
			//font->UpdateHeldLogger(this->logger);
			//if (font->Init(this->current_load_path + name) != 0)
			//{
			//	return nullptr;
			//}
			//
			//this->fonts.emplace(name, font);
			//return font;
		}
	}

	std::shared_ptr<Scripting::LuaScript> AssetManager::GetLuaScript(std::string name)
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "LuaScript %s requested", name.c_str());
		if (this->luascripts.find(name) != this->luascripts.end())
		{
			return this->luascripts.at(name);
		}
		else
		{
			return nullptr;
		}
	}

	std::shared_ptr<Rendering::Mesh> AssetManager::GetModel(std::string name)
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Model %s requested", name.c_str());
		if (this->models.find(name) != this->models.end())
		{
			return this->models.at(name);
		}
		else
		{
			this->logger->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "Model %s requested and is not loaded", name.c_str());
			this->AddModel(name, name);
			return nullptr;
		}
	}
#pragma endregion
}