#include "Assets/AssetManager.hpp"

#include "Assets/Font.hpp"
#include "Assets/Mesh.hpp"

#include "Logging/Logging.hpp"

#include "InternalEngineObject.hpp"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_ASSET_MANAGER
#include "Logging/LoggingPrefix.hpp"

namespace Engine
{
	AssetManager::AssetManager(Engine* with_engine) : InternalEngineObject(with_engine)
	{
		this->font_factory	  = std::make_unique<Factories::FontFactory>(with_engine);
		this->texture_factory = std::make_unique<Factories::TextureFactory>(with_engine);
	}

	AssetManager::~AssetManager()
	{
		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Destructor called");

		this->fonts.clear();

		for (auto& texture : this->textures)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Debug3,
				LOGPFX_CURRENT "Texture '%s' use_count: %u",
				texture.first.c_str(),
				texture.second.use_count()
			);
			texture.second.reset();
		}

		this->textures.clear();
	}

#pragma region Adding Assets
	void AssetManager::AddTexture(
		std::string name,
		const std::string& path,
		Rendering::Texture_InitFiletype filetype,
		VkFilter filtering,
		VkSamplerAddressMode address_mode
	)
	{
		std::shared_ptr<Rendering::Texture> texture =
			this->texture_factory->InitFile(path, nullptr, filetype, filtering, address_mode);

		this->textures.emplace(name, texture);
		this->logger->SimpleLog(
			Logging::LogLevel::Debug2,
			LOGPFX_CURRENT "Added texture '%s' as '%s'",
			path.c_str(),
			name.c_str()
		);
	}

	void AssetManager::AddFont(const std::string& name, const std::string& path)
	{
		std::shared_ptr<Rendering::Font> font = this->font_factory->InitBMFont(path);
		this->fonts.emplace(name, std::move(font));
	}

	void AssetManager::AddLuaScript(const std::string& name, const std::string& path)
	{
		auto* script = new Scripting::LuaScript(this->lua_executor, path);

		this->luascripts.emplace(name, script);
	}

	void AssetManager::AddModel(const std::string& name, const std::string& path)
	{
		std::shared_ptr<Rendering::Mesh> mesh = std::make_shared<Rendering::Mesh>(this->owner_engine);

		mesh->CreateMeshFromObj(path);

		this->models.emplace(name, std::move(mesh));
	}
#pragma endregion

#pragma region Getting Assets
	std::shared_ptr<Rendering::Texture> AssetManager::GetTexture(const std::string& name)
	{
		if (this->textures.find(name) != this->textures.end())
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Debug2,
				LOGPFX_CURRENT "Texture '%s' requested and is loaded",
				name.c_str()
			);
			return this->textures.at(name);
		}

		this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Texture asset '%s' not found", name.c_str());

		return nullptr;
	}

	std::shared_ptr<Rendering::Font> AssetManager::GetFont(const std::string& name)
	{
		if (this->fonts.find(name) != this->fonts.end())
		{
			return this->fonts.at(name);
		}

		this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Font asset '%s' not found", name.c_str());

		return nullptr;
	}

	std::shared_ptr<Scripting::LuaScript> AssetManager::GetLuaScript(const std::string& name)
	{
		if (this->luascripts.find(name) != this->luascripts.end())
		{
			return this->luascripts.at(name);
		}

		this->logger
			->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "LuaScript asset '%s' not found", name.c_str());

		return nullptr;
	}

	std::shared_ptr<Rendering::Mesh> AssetManager::GetModel(const std::string& name)
	{
		if (this->models.find(name) != this->models.end())
		{
			return this->models.at(name);
		}

		this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Model asset '%s' not found", name.c_str());

		return nullptr;
	}
#pragma endregion
} // namespace Engine
