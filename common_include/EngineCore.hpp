#pragma once

#include <cstdio>
#include <memory>
#include <string>

#define CMEP_ABI_IMPORT
#include "PlatformSemantics.hpp"

namespace Logging
{
	enum class LogLevel
	{
		Debug3	  = 0,
		Debug2	  = 1,
		Debug1	  = 2,
		Info	  = 3,
		Warning	  = 4,
		Error	  = 5,
		Exception = 6
	};

	class LoggerInternalState;

	class CMEP_EXPORT_CLASS Logger
	{
	private:
		LoggerInternalState* state;

	public:
		CMEP_EXPORT Logger();
		CMEP_EXPORT ~Logger();

		void CMEP_EXPORT AddOutputHandle(LogLevel min_level, FILE* handle, bool use_colors = false);
		void CMEP_EXPORT MapCurrentThreadToName(std::string name);

		void CMEP_EXPORT StartLog(LogLevel level);
		void CMEP_EXPORT Log(const char* format, ...);
		void CMEP_EXPORT StopLog();

		void CMEP_EXPORT SimpleLog(LogLevel level, const char* format, ...);
	};
} // namespace Logging

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

		[[noreturn]] CMEP_EXPORT void ThrowTest();

		CMEP_EXPORT void Init();
		CMEP_EXPORT void Run();

		CMEP_EXPORT void ConfigFile(const std::string& path);
	};

} // namespace Engine
