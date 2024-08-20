#include "Assets/Texture.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"

#include <cassert>

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_TEXTURE
#include "Logging/LoggingPrefix.hpp" // IWYU pragma: keep

namespace Engine::Rendering
{
	Texture::Texture(Engine* with_engine, std::unique_ptr<TextureData> init_data)
		: InternalEngineObject(with_engine), data(std::move(init_data))
	{
	}

	Texture::~Texture()
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Destructor called");

		Vulkan::Instance* instance = this->owner_engine->GetVulkanInstance();

		instance->GetLogicalDevice()->GetHandle().waitIdle();

		delete this->data->texture_image;

		this->data.reset();
	}

} // namespace Engine::Rendering
