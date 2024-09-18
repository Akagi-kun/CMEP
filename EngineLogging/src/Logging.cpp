#include <format>
#define ENGINELOGGING_LIBRARY_IMPLEMENTATION
#include "ConsoleColors.hpp"
#include "Logging.hpp"

#if defined(__GNUC__) || defined(__llvm__)
/**
 * @brief For compatibility with clang, use on functions taking vprintf-like arguments
 */
#	define ATTRIBUTE_PRINTF_COMPAT(string_idx, arg_check)                               \
		__attribute__((__format__(__printf__, string_idx, arg_check)))
#else
#	define ATTRIBUTE_PRINTF_COMPAT(string_idx, arg_check)
#endif

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <string>
#include <string_view>
#include <thread>

namespace Logging
{
	namespace
	{
		using color_t = std::string_view;
		using level_t = std::string_view;

		constexpr color_t level_to_color_table[] = {
			Logging::Console::GRAY_FG,
			Logging::Console::GRAY_FG,
			Logging::Console::GRAY_FG,
			Logging::Console::WHITE_FG,
			Logging::Console::YELLOW_FG,
			Logging::Console::RED_FG,
			Logging::Console::BLUE_FG
		};

		constexpr level_t level_to_string_table[] =
			{"???", "VDEBUG", "DEBUG", "INFO", "WARN", "ERROR", "EXCEPT"};

		static_assert(
			(sizeof(level_to_color_table) / sizeof(color_t)) ==
			(sizeof(level_to_string_table) / sizeof(level_t))
		);

		/**
		 * @brief Get the maximum length of level string
		 *        or the minimum length necessary to contain every possible level
		 */
		consteval size_t getMaxLevelLength()
		{
			size_t accum = 0;

			for (auto level : level_to_string_table)
			{
				accum = std::max(accum, level.size());
			}

			return accum;
		}

		/**
		 * @brief Get a 64bit unsigned hash of the current thread ID
		 */
		uint64_t getCurrentThreadId()
		{
			return std::hash<std::thread::id>{}(std::this_thread::get_id());
		}

		constexpr bool isValid(Logging::LogLevel level)
		{
			return level >= LogLevel::VerboseDebug && level <= LogLevel::Exception;
		}

		std::string getThreadOutputName(LoggerInternalState* state)
		{
			const uint64_t current_thread = getCurrentThreadId();

			auto find_result = state->threadid_name_map.find(current_thread);

			// If this ID is mapped to a name
			bool use_name = (find_result != state->threadid_name_map.end());

			// Use the name, else format the ID in hexadecimal
			auto output = use_name ? find_result->second
								   : std::format("{:X}", current_thread);

			// Limit size to 8 characters
			if (output.length() > 8) { return output.substr(output.length() - 8, 8); }

			return output;
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
			if (output.handle != stdout) { fclose(output.handle); }
		}

		delete state;
	}

	void
	Logger::addOutputHandle(Logging::LogLevel min_level, FILE* handle, bool use_colors)
	{
		if (!isValid(min_level)) { return; }

		// Create new mapping
		LoggerInternalMapping new_map = LoggerInternalMapping();
		new_map.min_level			  = min_level;
		new_map.handle				  = handle;
		new_map.has_started_logging	  = false;
		new_map.use_colors			  = use_colors;

		// Add new mapping to list
		state->outputs.push_back(new_map);
	}

	void Logger::mapCurrentThreadToName(std::string name)
	{
		assert(name.length() <= 8);

		// Protect member access
		state->thread_mutex.lock();

		uint64_t thread_id = getCurrentThreadId();

		state->threadid_name_map.emplace(thread_id, name);

		state->thread_mutex.unlock();
	}

	void Logger::internalStartLog(Logging::LogLevel level, const char* log_prefix)
	{
		if (!isValid(level)) { return; }

		// Protect IO and member access
		state->thread_mutex.lock();

		// Get color and string representation of LogLevel
		auto color_str = level_to_color_table[static_cast<int>(level)];
		auto level_str = level_to_string_table[static_cast<int>(level)];

		// Get current time
		const auto system_time = std::chrono::time_point_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now()
		);

		std::string thread_name = getThreadOutputName(state);

		// For all outputs
		for (auto& output : state->outputs)
		{
			// Log only if selected level is higher than output's minimum
			if (level >= output.min_level)
			{
				output.has_started_logging = true;

				const auto out = std::format(
					"{}{:%T} {:>8} {}{:{}} | {}: ",
					output.use_colors ? Console::GRAY_FG : "",
					system_time,
					thread_name,
					output.use_colors ? color_str : "",
					level_str,
					getMaxLevelLength(),
					log_prefix != nullptr ? log_prefix : logpfx_generator<void>{}
				);

				fputs(out.c_str(), output.handle);
			}
		}
		assert(thread_name.length() <= 8);
	}

	void Logger::log(const char* format, ...)
	{
		assert(format != nullptr);

		va_list args;
		va_start(args, format);
		internalLog(format, args);
		va_end(args);
	}

	void Logger::stopLog()
	{
		for (auto& output : state->outputs)
		{
			if (output.has_started_logging)
			{
				output.has_started_logging = false;
				if (output.use_colors)
				{
					fputs(Logging::Console::RESET_FG.data(), output.handle);
				}
				fputc('\n', output.handle);
			}
		}

		// All logging must end with StopLog, unlock mutex here
		state->thread_mutex.unlock();
	}

	ATTRIBUTE_PRINTF_COMPAT(2, 0)
	void Logger::internalLog(const char* const format, va_list args)
	{
		for (auto& output : state->outputs)
		{
			if (output.has_started_logging) { vfprintf(output.handle, format, args); }
		}
	}
} // namespace Logging
