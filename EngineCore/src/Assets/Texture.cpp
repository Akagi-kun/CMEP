#include "Assets/Texture.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"
#include "InternalEngineObject.hpp"

#include <cassert>
#include <memory>
#include <utility>

namespace Engine::Rendering
{
	Texture::Texture(Engine* with_engine, std::unique_ptr<TextureData> init_data)
		: InternalEngineObject(with_engine), data(std::move(init_data))
	{
	}

	Texture::~Texture()
	{
		this->logger->logSingle<decltype(this)>(Logging::LogLevel::VerboseDebug, "Destructor called");

		delete this->data->image;
		delete this->data->sampler;

		this->data.reset();
	}

} // namespace Engine::Rendering
