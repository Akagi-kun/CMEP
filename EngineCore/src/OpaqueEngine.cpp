#include "OpaqueEngine.hpp"

#include "Engine.hpp"

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

	void OpaqueEngine::Init()
	{
		d_engine->Init();
	}

	[[noreturn]] void OpaqueEngine::ThrowTest()
	{
		d_engine->ThrowTest();
	}

	void OpaqueEngine::Run()
	{
		d_engine->Run();
	}

	void OpaqueEngine::ConfigFile(const char* path)
	{
		d_engine->ConfigFile(path);
	}
} // namespace Engine
