#include "Logging/Logging.hpp"

#include "Logging/ConsoleColors.hpp"

#include <chrono>
#include <cstdarg>
#include <stdio.h>
#include <thread>

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_LOGGER
#include "Logging/LoggingPrefix.hpp"

static const char* level_to_color_table[] = {
	Logging::Console::GRAY_FG,
	Logging::Console::GRAY_FG,
	Logging::Console::GRAY_FG,
	Logging::Console::WHITE_FG,
	Logging::Console::YELLOW_FG,
	Logging::Console::RED_FG,
	Logging::Console::BLUE_FG
};

static const char* const level_to_string_table[] = {"DBG3", "DBG2", "DBG1", "INFO", "WARN", "ERROR", "EXCEPTION"};

// static constexpr size_t level_count =
//	std::min(sizeof(level_to_color_table) / sizeof(char*), sizeof(level_to_string_table) / sizeof(char*));

namespace Logging
{
	Logger::Logger()
	{
		this->state = new LoggerInternalState();
	}

	Logger::~Logger()
	{
		// Close all handles
		for (auto& output : this->state->outputs)
		{
			if (output.handle != stdout)
			{
				fclose(output.handle);
			}
		}

		delete this->state;
	}

	void Logger::AddOutputHandle(Logging::LogLevel min_level, FILE* handle, bool use_colors)
	{
		// Check if enum valid
		if (min_level < LogLevel::Debug3 || min_level > LogLevel::Exception)
		{
			return;
		}

		// Create new mapping
		LoggerInternalMapping new_map = LoggerInternalMapping();
		new_map.min_level			  = min_level;
		new_map.handle				  = handle;
		new_map.has_started_logging	  = false;
		new_map.use_colors			  = use_colors;

		// Add new mapping to list
		this->state->outputs.push_back(new_map);
	}

	static inline uint16_t GetCurrentThreadID()
	{
		static const uint16_t thread_id_mask = 0xFFFF;
		return static_cast<uint16_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()) & thread_id_mask);
	}

	void Logger::MapCurrentThreadToName(std::string name)
	{
		// Protect member access
		this->state->thread_mutex.lock();

		uint16_t thread_id = GetCurrentThreadID();

		this->state->threadid_name_map.emplace(thread_id, name);

		this->state->thread_mutex.unlock();
	}

	void Logger::StartLog(Logging::LogLevel level)
	{
		// Check if enum valid
		if (level < LogLevel::Debug3 || level > LogLevel::Exception)
		{
			return;
		}

		// Protect IO and member access
		this->state->thread_mutex.lock();

		// Get color and string representation of LogLevel
		const char* const color_str = level_to_color_table[static_cast<int>(level)];
		const char* const level_str = level_to_string_table[static_cast<int>(level)];

		// Get current time
		const std::time_t tmp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::tm cur_time	  = {};
#if defined(_MSC_VER)
		localtime_s(&cur_time, &tmp);
// TODO: Check this macro
#else
		localtime_r(&tmp, &cur_time);
#endif

		// StartLog for all outputs
		for (auto& output : this->state->outputs)
		{
			// Log only if level is higher than outputs filter
			if (level >= output.min_level)
			{
				output.has_started_logging = true;

				uint16_t thread_id = GetCurrentThreadID();

				auto find_result = this->state->threadid_name_map.find(thread_id);

				fprintf(
					output.handle,
					"%s[%02i:%02i:%02i ",
					output.use_colors ? color_str : "",
					cur_time.tm_hour,
					cur_time.tm_min,
					cur_time.tm_sec
				);

				// TODO: Fix
				if (find_result == this->state->threadid_name_map.end())
				{
					fprintf(output.handle, "%04hx %s] ", thread_id, level_str);
				}
				else
				{
					fprintf(output.handle, "%s %s] ", find_result->second.c_str(), level_str);
				}
			}
		}
	}

	void Logger::Log(const char* format, ...)
	{
		// Log for all outputs
		for (auto& output : this->state->outputs)
		{
			if (output.has_started_logging)
			{
				va_list args;
				va_start(args, format);
				vfprintf(output.handle, format, args);
				va_end(args);
			}
		}
	}

	void Logger::StopLog()
	{
		// StopLog for all outputs
		for (auto& output : this->state->outputs)
		{
			if (output.has_started_logging)
			{
				output.has_started_logging = false;
				if (output.use_colors)
				{
					fprintf(output.handle, "%s", Logging::Console::WHITE_FG);
				}
				fputc('\n', output.handle);
			}
		}

		// All logging must end with StopLog, unlock mutex here
		this->state->thread_mutex.unlock();
	}

	void Logger::SimpleLog(LogLevel level, const char* format, ...)
	{
		this->StartLog(level);

		// Log for all outputs
		for (auto& output : this->state->outputs)
		{
			if (level >= output.min_level)
			{
				va_list args;
				va_start(args, format);
				vfprintf(output.handle, format, args);
				va_end(args);
			}
		}

		this->StopLog();
	}
} // namespace Logging
