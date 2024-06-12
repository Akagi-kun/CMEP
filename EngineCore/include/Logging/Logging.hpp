#pragma once

#include <cstdio>
#include <map>
#include <mutex>
#include <string>
#include <vector>
// #include <atomic>

#include "PlatformSemantics.hpp"

namespace Logging
{
	enum class LogLevel
	{
		Debug3 = 0,
		Debug2,
		Debug1,
		Info,
		Warning,
		Error,
		Exception
	};

	struct LoggerInternalMapping
	{
		LogLevel min_level;
		FILE* handle;
		bool has_started_logging;
		bool use_colors;
	};

	// TODO: Check thread safety
	class CMEP_EXPORT_CLASS Logger
	{
	private:
		std::vector<LoggerInternalMapping*> outputs;
		std::map<int16_t, std::string> threadid_name_map;
		std::mutex thread_mutex;

	public:
		CMEP_EXPORT Logger() = default;
		CMEP_EXPORT ~Logger()
		{
		}

		void CMEP_EXPORT AddOutputHandle(LogLevel min_level, FILE* handle, bool useColors = false);
		void CMEP_EXPORT MapCurrentThreadToName(std::string name);

		void CMEP_EXPORT StartLog(LogLevel level);
		void CMEP_EXPORT Log(const char* format, ...);
		void CMEP_EXPORT StopLog();

		void CMEP_EXPORT SimpleLog(LogLevel level, const char* format, ...);
	};
} // namespace Logging
