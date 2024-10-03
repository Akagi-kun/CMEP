#pragma once
// IWYU pragma: private; include Logging/Logging.hpp

#include "PlatformSemantics.hpp"

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

namespace Logging
{
	enum class LogLevel : uint8_t
	{
		// 0 is invalid

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
	/**
	 * @brief The type internally responsible for storage of logging prefixes
	 */
	template <typename noptr_class_t> struct CMEP_EXPORT_CLASS logpfx_generator_internal
	{
		CMEP_EXPORT static const char* value;
	};

	/**
	 * @brief Utility struct that generates a logging prefix
	 *
	 * @tparam class_t Type representing the caller (i.e. a method's class),
	 *                 generates an empty string if this is void.
	 */
	template <typename class_t> struct logpfx_generator
	{
		using remptr_t = std::remove_pointer_t<class_t>;
		using prefix_t = decltype(logpfx_generator_internal<remptr_t>::value);

		/**
		 * @return The prefix as a string
		 */
		operator prefix_t() const
		{
			return logpfx_generator_internal<remptr_t>::value;
		}
	};

	class CMEP_EXPORT_CLASS Logger
	{
	public:
		Logger();
		~Logger();

		void addOutputHandle(LogLevel min_level, FILE* handle, bool use_colors = false);
		void mapCurrentThreadToName(std::string name);

		template <typename class_t> void startLog(LogLevel level)
		{
			internalStartLog(level, logpfx_generator<class_t>{});
		}
		void log(const char* format, ...);
		void stopLog();

		template <typename class_t>
		void simpleLog(LogLevel level, const char* format, ...)
		{
			startLog<class_t>(level);

			va_list args;
			va_start(args, format);
			internalLog(format, args);
			va_end(args);

			stopLog();
		}

	private:
		LoggerInternalState* state;

		void internalStartLog(LogLevel level, const char* log_prefix);

		void internalLog(const char* format, va_list args);
	};

	class CMEP_EXPORT_CLASS SupportsLogging
	{
	public:
		using logger_t = std::shared_ptr<Logging::Logger>;

		SupportsLogging() = delete;
		CMEP_EXPORT SupportsLogging(logger_t with_logger) : logger(std::move(with_logger))
		{}

		[[nodiscard]] CMEP_EXPORT logger_t getLogger()
		{
			return logger;
		}

	protected:
		logger_t logger;
	};
} // namespace Logging
