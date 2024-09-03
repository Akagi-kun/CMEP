#pragma once

#include "PlatformSemantics.hpp"

#include <cstdarg>
#include <cstdio>
#include <memory>
#include <string>

namespace Logging
{
	enum class LogLevel : uint8_t
	{
		// Debug3 = 0,

		// Information only useful for debugging the engine itself
		// (constructors/destructors being called, stuff being initialized etc.)
		//
		// verbose if debug build
		VerboseDebug = 1,

		// Information potentially useful for users and game developers
		// when resolving problems with an installation or game
		//
		// default if debug build
		// verbose if release build
		Debug = 2,

		Info	  = 3,
		Warning	  = 4,
		Error	  = 5,
		Exception = 6
	};
} // namespace Logging

namespace Logging
{
	struct LoggerInternalState;
}

#ifdef ENGINELOGGING_LIBRARY_IMPLEMENTATION
#	include <map>
#	include <mutex>
#	include <vector>

namespace Logging
{
	struct LoggerInternalMapping
	{
		LogLevel min_level;
		FILE*	 handle;
		bool	 has_started_logging;
		bool	 use_colors;
	};

	struct LoggerInternalState
	{
		std::vector<LoggerInternalMapping> outputs;
		std::map<uint64_t, std::string>	   threadid_name_map;
		std::mutex						   thread_mutex;
	};

} // namespace Logging
#endif

namespace Logging
{
	template <typename noptr_class_t> struct CMEP_EXPORT_CLASS prefix_internal
	{
		CMEP_EXPORT static const char* value;
	};

	template <typename class_t> struct CMEP_EXPORT_CLASS logpfx_generator
	{
		using remptr_t = std::remove_pointer_t<class_t>;
		using prefix_t = decltype(prefix_internal<remptr_t>::value);

		CMEP_EXPORT operator prefix_t() const
		{
			return prefix_internal<remptr_t>{}.value;
		}
	};

	class CMEP_EXPORT_CLASS Logger
	{
	public:
		Logger();
		~Logger();

		void AddOutputHandle(LogLevel min_level, FILE* handle, bool use_colors = false);
		void MapCurrentThreadToName(std::string name);

		template <typename class_t> void StartLog(LogLevel level)
		{
			InternalStartLog(level, logpfx_generator<class_t>{});
		}
		void Log(const char* format, ...);
		void StopLog();

		template <typename class_t> void SimpleLog(LogLevel level, const char* format, ...)
		{
			StartLog<class_t>(level);

			va_list args;
			va_start(args, format);
			InternalLog(format, args);
			va_end(args);

			StopLog();
		}

	private:
		LoggerInternalState* state;

		void InternalStartLog(LogLevel level, const char* log_prefix);

		void InternalLog(const char* format, va_list args);
	};

	class CMEP_EXPORT_CLASS SupportsLogging
	{
	public:
		using logger_t = std::shared_ptr<Logging::Logger>;

		SupportsLogging() = delete;
		CMEP_EXPORT SupportsLogging(logger_t with_logger) : logger(std::move(with_logger))
		{
		}

		[[nodiscard]] CMEP_EXPORT logger_t GetLogger()
		{
			return logger;
		}

	protected:
		logger_t logger;
	};
} // namespace Logging
