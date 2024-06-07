#pragma once

#include <atomic>
#include <map>
#include <memory>
#include <optional>
#include <vector>
#include <mutex>
#include <stdio.h>

#define CMEP_ABI_IMPORT
#include "PlatformSemantics.hpp"

#pragma region forward decls

namespace Logging
{
	enum class LogLevel
	{
		Debug3 = 0,
		Debug2,
		Debug1,
		Info,
		Success,
		Warning,
		Error,
		Exception
	};

	struct LoggerInternalMapping;

	class CMEP_EXPORT_CLASS Logger
	{
	private:
		std::vector<LoggerInternalMapping*> outputs;
		std::map<int16_t, std::string> threadid_name_map;
		std::mutex threadMutex{};

	public:
		CMEP_EXPORT Logger() {}
		CMEP_EXPORT ~Logger() {};

		void CMEP_EXPORT AddOutputHandle(LogLevel min_level, FILE* handle, bool useColors = false);
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
		CMEP_EXPORT OpaqueEngine(std::shared_ptr<Logging::Logger> logger) noexcept;
		CMEP_EXPORT ~OpaqueEngine() noexcept;

		CMEP_EXPORT void Init();
		CMEP_EXPORT void Run();

		CMEP_EXPORT void ConfigFile(std::string path);
	};

} // namespace Engine
