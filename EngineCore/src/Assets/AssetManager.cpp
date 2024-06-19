#include "Assets/AssetManager.hpp"

#include "Assets/Font.hpp"
#include "Assets/Mesh.hpp"

#include "Logging/Logging.hpp"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_ASSET_MANAGER
#include "Logging/LoggingPrefix.hpp"

namespace Engine
{
	AssetManager::AssetManager()
	{
		this->font_factory = std::make_unique<Factories::FontFactory>(this);
		this->texture_factory = std::make_unique<Factories::TextureFactory>();
	}

	AssetManager::~AssetManager()
	{
		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Destructor called");

		this->fonts.clear();

		for (auto& texture : this->textures)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Debug3,
				LOGPFX_CURRENT "Texture %s use_count: %u",
				texture.first.c_str(),
				texture.second.use_count()
			);
			texture.second.reset();
		}

		this->textures.clear();
		this->texture_factory.reset();
		this->font_factory.reset();
	}

	void AssetManager::UpdateHeldLogger(std::shared_ptr<Logging::Logger> new_logger)
	{
		InternalEngineObject::UpdateHeldLogger(new_logger);
		this->font_factory->UpdateHeldLogger(new_logger);
		this->texture_factory->UpdateHeldLogger(new_logger);
	}

	void AssetManager::UpdateOwnerEngine(Engine* new_owner_engine)
	{
		InternalEngineObject::UpdateOwnerEngine(new_owner_engine);
		this->font_factory->UpdateOwnerEngine(new_owner_engine);
		this->texture_factory->UpdateOwnerEngine(new_owner_engine);
	}

#pragma region Adding Assets
	void AssetManager::AddTexture(
		std::string name,
		std::string path,
		Rendering::Texture_InitFiletype filetype,
		VkFilter filtering,
		VkSamplerAddressMode address_mode
	)
	{
		std::shared_ptr<Rendering::Texture> texture = this->texture_factory->InitFile(
			path, nullptr, filetype, filtering, address_mode, 0, 0
		);

		this->textures.emplace(name, texture);
		this->logger->SimpleLog(
			Logging::LogLevel::Debug2, LOGPFX_CURRENT "Added texture '%s' as '%s'", path.c_str(), name.c_str()
		);
	}

	void AssetManager::AddFont(std::string name, std::string path)
	{
		std::shared_ptr<Rendering::Font> font = this->font_factory->InitBMFont(std::move(path));
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

		mesh->CreateMeshFromObj(path);

		this->models.emplace(name, std::move(mesh));
	}
#pragma endregion

#pragma region Getting Assets
	std::shared_ptr<Rendering::Texture> AssetManager::GetTexture(std::string name)
	{
		if (this->textures.find(name) != this->textures.end())
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Debug2, LOGPFX_CURRENT "Texture %s requested and is loaded", name.c_str()
			);
			return this->textures.at(name);
		}
		else
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Error, LOGPFX_CURRENT "Texture asset '%s' not found", name.c_str()
			);
			// this->logger->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "Texture %s requested and is not
			// loaded", name.c_str()); this->AddTexture(name, name, Rendering::Texture_InitFiletype::FILE_PNG);

			// return this->GetTexture(name);
			return nullptr;
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
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Font asset '%s' not found", name.c_str());

			// TODO: Recursive!!!
			// this->AddFont(name, name);
			// return this->GetFont(name);
			return nullptr;

			// std::shared_ptr<Rendering::Font> font = std::make_shared<Rendering::Font>(this);
			//
			// font->UpdateHeldLogger(this->logger);
			// if (font->Init(this->current_load_path + name) != 0)
			//{
			//	return nullptr;
			// }
			//
			// this->fonts.emplace(name, font);
			// return font;
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
			this->logger->SimpleLog(
				Logging::LogLevel::Error, LOGPFX_CURRENT "LuaScript asset '%s' not found", name.c_str()
			);
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
			this->logger->SimpleLog(
				Logging::LogLevel::Error, LOGPFX_CURRENT "Model asset '%s' not found", name.c_str()
			);
			// this->logger->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "Model %s requested and is not loaded",
			// name.c_str());
			this->AddModel(name, name);
			return nullptr;
		}
	}
#pragma endregion
} // namespace Engine
