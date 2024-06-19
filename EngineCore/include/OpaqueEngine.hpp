#include "PlatformSemantics.hpp"

#include <memory>
#include <string>

namespace Logging
{
	class Logger;
}

namespace Engine
{
	class Engine;

	class CMEP_EXPORT_CLASS OpaqueEngine
	{
	private:
		std::unique_ptr<Engine> d_engine;

	public:
		CMEP_EXPORT OpaqueEngine(std::shared_ptr<Logging::Logger>& logger) noexcept;
		CMEP_EXPORT ~OpaqueEngine() noexcept;

		CMEP_EXPORT void Init();
		CMEP_EXPORT void Run();

		CMEP_EXPORT void ConfigFile(std::string path);
	};
} // namespace Engine
