#include "Logging/Logging.hpp"
#include "Logging/ConsoleColors.hpp"

#include <chrono>
#include <cstdarg>
#include <exception>
#include <stdio.h>

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_LOGGER
#include "Logging/LoggingPrefix.hpp"

static const char* level_to_color_table[7] = {
	Logging::Console::GRAY_FG,
	Logging::Console::GRAY_FG,
	Logging::Console::GRAY_FG,
	Logging::Console::WHITE_FG,
	Logging::Console::YELLOW_FG,
	Logging::Console::RED_FG,
	Logging::Console::BLUE_FG
};

static const char* level_to_string_table[7] = {"DBG3", "DBG2", "DBG1", "INFO", "WARN", "ERROR", "EXCEPTION"};

void Logging::Logger::AddOutputHandle(Logging::LogLevel min_level, FILE* handle, bool use_colors)
{
	// Check if enum valid
	if (min_level < LogLevel::Debug3 || min_level > LogLevel::Exception)
	{
		return;
	}

	// Create new mapping
	LoggerInternalMapping* new_map = new LoggerInternalMapping();
	new_map->min_level = min_level;
	new_map->handle = handle;
	new_map->has_started_logging = false;
	new_map->use_colors = use_colors;

	// Add new mapping to list
	this->outputs.push_back(new_map);
}

void Logging::Logger::MapCurrentThreadToName(std::string name)
{
	int16_t thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id()) & 0xFFFF;

	// Protect member access
	this->thread_mutex.lock();

	this->threadid_name_map.emplace(thread_id, name);

	this->thread_mutex.unlock();
}

void Logging::Logger::StartLog(Logging::LogLevel level)
{
	// Check if enum valid
	if (level < LogLevel::Debug3 || level > LogLevel::Exception)
	{
		return;
	}

	// Protect IO and member access
	this->thread_mutex.lock();

	// StartLog for all outputs
	for (auto output : this->outputs)
	{
		// Log only if level is higher than outputs filter
		if (level >= output->min_level)
		{
			output->has_started_logging = true;

			// Get current time
			const std::time_t tmp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			std::tm cur_time = {};
#if defined(_MSC_VER)
			localtime_s(&cur_time, &tmp);
#elif defined(_GNUC_)
			localtime_r(&cur_time, &tmp);
#endif

			int16_t thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id()) & 0xFFFF;
			auto find_result = this->threadid_name_map.find(thread_id);

			if (find_result == this->threadid_name_map.end())
			{
				fprintf(
					output->handle,
					"%s[%02i:%02i:%02i %04hx %s] ",
					output->use_colors ? level_to_color_table[static_cast<int>(level)] : "",
					cur_time.tm_hour,
					cur_time.tm_min,
					cur_time.tm_sec,
					thread_id,
					level_to_string_table[static_cast<int>(level)]
				);
			}
			else
			{
				fprintf(
					output->handle,
					"%s[%02i:%02i:%02i %s %s] ",
					output->use_colors ? level_to_color_table[static_cast<int>(level)] : "",
					cur_time.tm_hour,
					cur_time.tm_min,
					cur_time.tm_sec,
					find_result->second.c_str(),
					level_to_string_table[static_cast<int>(level)]
				);
			}
		}
	}
}

void Logging::Logger::Log(const char* format, ...)
{
	// Log for all outputs
	for (auto output : this->outputs)
	{
		if (output->has_started_logging)
		{
			va_list args;
			va_start(args, format);
			vfprintf(output->handle, format, args);
			va_end(args);
		}
	}
}

void Logging::Logger::StopLog()
{
	// StopLog for all outputs
	for (auto output : this->outputs)
	{
		if (output->has_started_logging)
		{
			output->has_started_logging = false;
			if (output->use_colors)
			{
				fprintf(output->handle, "%s", Logging::Console::WHITE_FG);
			}
			fputc('\n', output->handle);
		}
	}

	// All logging must end with StopLog, unlock mutex here
	this->thread_mutex.unlock();
}

void Logging::Logger::SimpleLog(LogLevel level, const char* format, ...)
{
	this->StartLog(level);

	// Log for all outputs
	for (auto output : this->outputs)
	{
		if (level >= output->min_level)
		{
			va_list args;
			va_start(args, format);
			vfprintf(output->handle, format, args);
			va_end(args);
		}
	}

	this->StopLog();
}
