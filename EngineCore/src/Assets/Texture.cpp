#include "Assets/Texture.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"

#include <cassert>

namespace Engine::Rendering
{
	Texture::Texture(Engine* with_engine, std::unique_ptr<TextureData> init_data)
		: InternalEngineObject(with_engine), data(std::move(init_data))
	{
	}

	Texture::~Texture()
	{
		this->logger->simpleLog<decltype(this)>(
			Logging::LogLevel::VerboseDebug,
			"Destructor called"
		);

		delete this->data->texture_image;

		this->data.reset();
	}

} // namespace Engine::Rendering
