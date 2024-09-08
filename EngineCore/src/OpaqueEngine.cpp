#include "OpaqueEngine.hpp"

#include "Engine.hpp"

#include <memory>

namespace Engine
{
	OpaqueEngine::OpaqueEngine(std::shared_ptr<Logging::Logger>& logger) noexcept
	{
		d_engine = std::make_unique<Engine>(logger);
	}

	OpaqueEngine::~OpaqueEngine() noexcept
	{
		d_engine.reset();
	}

	[[noreturn]] void OpaqueEngine::throwTest()
	{
		d_engine->throwTest();
	}

	void OpaqueEngine::run()
	{
		d_engine->run();
	}

	void OpaqueEngine::configFile(const char* path)
	{
		d_engine->configFile(path);
	}
} // namespace Engine
