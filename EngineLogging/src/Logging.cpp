#define ENGINELOGGING_LIBRARY_IMPLEMENTATION
#include "Logging.hpp"

#include "ConsoleColors.hpp"

#if defined(__GNUC__) || defined(__llvm__)
#	define ATTRIBUTE_PRINTF_COMPAT(string_idx, arg_check)                                         \
		__attribute__((__format__(__printf__, string_idx, arg_check)))
#else
#	define ATTRIBUTE_PRINTF_COMPAT(string_idx, arg_check)
#endif

#include <cassert>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <thread>

namespace
{
	using color_t = const char*;
	using level_t = std::string_view;

	const char* level_to_color_table[] = {
		Logging::Console::GRAY_FG,
		Logging::Console::GRAY_FG,
		Logging::Console::GRAY_FG,
		Logging::Console::WHITE_FG,
		Logging::Console::YELLOW_FG,
		Logging::Console::RED_FG,
		Logging::Console::BLUE_FG
	};

	constexpr std::string_view level_to_string_table[] =
		{"???", "VDEBUG", "DEBUG", "INFO", "WARN", "ERROR", "EXCEPT"};

	static_assert(
		(sizeof(level_to_color_table) / sizeof(color_t)) ==
		(sizeof(level_to_string_table) / sizeof(level_t))
	);
} // namespace

namespace Logging
{
	namespace
	{
		constexpr size_t GetMaxLevelLength()
		{
			size_t accum = 0;

			for (auto level : level_to_string_table)
			{
				accum = std::max(accum, level.size());
			}

			return accum;
		}

		uint64_t GetCurrentThreadID()
		{
			return std::hash<std::thread::id>{}(std::this_thread::get_id());
		}

		constexpr bool IsValid(Logging::LogLevel level)
		{
			return level >= LogLevel::VerboseDebug && level <= LogLevel::Exception;
		}
	} // namespace

	Logger::Logger()
	{
		state = new LoggerInternalState();
	}

	Logger::~Logger()
	{
		// Close all handles
		for (auto& output : state->outputs)
		{
			if (output.handle != stdout)
			{
				fclose(output.handle);
			}
		}

		delete state;
	}

	void Logger::AddOutputHandle(Logging::LogLevel min_level, FILE* handle, bool use_colors)
	{
		if (!IsValid(min_level))
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
		state->outputs.push_back(new_map);
	}

	void Logger::MapCurrentThreadToName(std::string name)
	{
		assert(name.length() <= 8);

		// Protect member access
		state->thread_mutex.lock();

		uint64_t thread_id = GetCurrentThreadID();

		state->threadid_name_map.emplace(thread_id, name);

		state->thread_mutex.unlock();
	}

	void Logger::InternalStartLog(Logging::LogLevel level, const char* log_prefix)
	{
		if (!IsValid(level))
		{
			return;
		}

		// Protect IO and member access
		state->thread_mutex.lock();

		// Get color and string representation of LogLevel
		const auto* color_str = level_to_color_table[static_cast<int>(level)];
		auto		level_str = level_to_string_table[static_cast<int>(level)];

		static constexpr size_t threadid_buf_len			   = 8 + 1;
		static char				threadid_buf[threadid_buf_len] = {};
		memset(threadid_buf, 0, threadid_buf_len);

		// Get current time
		const std::time_t tmp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now(
		));
		std::tm			  cur_time = {};
#if defined(_MSC_VER)
		localtime_s(&cur_time, &tmp);
#else
		localtime_r(&tmp, &cur_time);
#endif

		// For all outputs
		for (auto& output : state->outputs)
		{
			// Log only if selected level is higher than output's minimum
			if (level >= output.min_level)
			{
				output.has_started_logging = true;

				uint64_t thread_id		 = GetCurrentThreadID();
				auto	 find_result	 = state->threadid_name_map.find(thread_id);
				bool	 use_thread_name = (find_result != state->threadid_name_map.end());

				if (!use_thread_name)
				{
					snprintf(threadid_buf, threadid_buf_len, "%08llX", thread_id);
				}

				fprintf(
					output.handle,
					"%s%02i:%02i:%02i %8s %s%*s | %s: ",
					output.use_colors ? Console::GRAY_FG : "",
					cur_time.tm_hour,
					cur_time.tm_min,
					cur_time.tm_sec,
					use_thread_name ? find_result->second.c_str() : threadid_buf,
					output.use_colors ? color_str : "",
					static_cast<int>(GetMaxLevelLength()),
					level_str.data(),
					log_prefix != nullptr ? log_prefix : "no prefix"
				);
			}
		}
	}

	void Logger::Log(const char* format, ...)
	{
		assert(format != nullptr);

		va_list args;
		va_start(args, format);
		InternalLog(format, args);
		va_end(args);
	}

	void Logger::StopLog()
	{
		for (auto& output : state->outputs)
		{
			if (output.has_started_logging)
			{
				output.has_started_logging = false;
				if (output.use_colors)
				{
					fprintf(output.handle, "%s", Logging::Console::RESET_FG);
				}
				fputc('\n', output.handle);
			}
		}

		// All logging must end with StopLog, unlock mutex here
		state->thread_mutex.unlock();
	}

	ATTRIBUTE_PRINTF_COMPAT(2, 0)
	void Logger::InternalLog(const char* const format, va_list args)
	{
		for (auto& output : state->outputs)
		{
			if (output.has_started_logging)
			{
				vfprintf(output.handle, format, args);
			}
		}
	}
} // namespace Logging
