#include "OpaqueEngine.hpp"

#include "Engine.hpp"

namespace Engine
{
	OpaqueEngine::OpaqueEngine(std::shared_ptr<Logging::Logger> logger) noexcept
	{
		this->d_engine = std::make_unique<Engine>(logger);
	}

	OpaqueEngine::~OpaqueEngine() noexcept
	{
		this->d_engine.reset();
	}

	void OpaqueEngine::Init()
	{
		this->d_engine->Init();
	}

	void OpaqueEngine::Run()
	{
		this->d_engine->Run();
	}

	void OpaqueEngine::ConfigFile(std::string path)
	{
		this->d_engine->ConfigFile(path);
	}
} // namespace Engine
