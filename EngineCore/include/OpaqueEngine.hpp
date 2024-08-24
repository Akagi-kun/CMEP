#include "PlatformSemantics.hpp"

#include <memory>

namespace Logging
{
	class Logger;
}

namespace Engine
{
	class Engine;

	class CMEP_EXPORT_CLASS OpaqueEngine
	{
	public:
		CMEP_EXPORT OpaqueEngine(std::shared_ptr<Logging::Logger>& logger) noexcept;
		CMEP_EXPORT ~OpaqueEngine() noexcept;

		[[noreturn]] CMEP_EXPORT void ThrowTest();

		CMEP_EXPORT void Init();
		CMEP_EXPORT void Run();

		CMEP_EXPORT void ConfigFile(const char* path);

	private:
		std::unique_ptr<Engine> d_engine;
	};
} // namespace Engine
