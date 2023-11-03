#pragma once

#include <cstdio>
#include <vector>
#include <map>
#include <atomic>
#include <string>
#include <thread>
#include <memory>

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

	struct LoggerInternalMapping
	{
		LogLevel min_level;
		FILE* handle;
		bool hasStartedLogging;
		bool useColors;
	};

	class Logger
	{
	private:
		std::vector<LoggerInternalMapping*> outputs;
		std::map<int16_t, std::string> threadid_name_map;
		std::atomic<bool> threadLocked;

	public:
		Logger() {}

		void AddOutputHandle(LogLevel min_level, FILE* handle, bool useColors = false);
		void MapCurrentThreadToName(std::string name);

		void StartLog(LogLevel level);
		void Log(const char* format, ...);
		void StopLog();

		void SimpleLog(LogLevel level, const char* format, ...);
	};

	extern std::unique_ptr<Logger> GlobalLogger;
}