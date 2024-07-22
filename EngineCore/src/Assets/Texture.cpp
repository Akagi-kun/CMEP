#include "Assets/Texture.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"

#include <cassert>

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_TEXTURE
#include "Logging/LoggingPrefix.hpp"

namespace Engine::Rendering
{
	Texture::Texture(Engine* with_engine, std::unique_ptr<TextureData> init_data)
		: InternalEngineObject(with_engine), data(std::move(init_data))
	{
	}

	Texture::~Texture()
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Destructor called");

		Vulkan::VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		renderer->SyncDeviceWaitIdle();

		delete this->data->texture_image;

		this->data.reset();
	}

} // namespace Engine::Rendering
