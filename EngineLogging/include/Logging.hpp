#pragma once
// IWYU pragma: private; include Logging/Logging.hpp

#include "PlatformSemantics.hpp"

#include <algorithm>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#ifdef ENGINELOGGING_LIBRARY_IMPLEMENTATION
#	include <map>
#	include <mutex>
#	include <vector>
#endif

namespace Logging
{
	enum class LogLevel : uint8_t
	{
		/**
		 * Information only useful for debugging the engine itself
		 * (e.g. constructors/destructors being called, stuff being initialized etc.)
		 */
		VerboseDebug = 1,
		/**
		 * Information potentially useful for users and game developers
		 * when resolving problems with an installation or game
		 */
		Debug		 = 2,
		/**
		 * Information that should always be printed out, even though mostly
		 * unimportant (e.g. state changed, loading finished)
		 */
		Info		 = 3,
		/**
		 * Something unexpected happened but the engine can continue running
		 * (e.g. an invalid input but can fall back to a default)
		 */
		Warning		 = 4,
		/**
		 * A possibly critical event occured that is unlikely to be recovered from
		 * but the engine will try to continue
		 */
		Error		 = 5,
		/**
		 * Events that cannot be recovered from, should preferably unroll exception
		 * trace and exit out immediately.
		 */
		Exception	 = 6,

		MinValid = VerboseDebug,
		MaxValid = Exception
	};

	/**
	 * Adds support for substraction to @ref LogLevel
	 *
	 * @warning Result is not checked, use @ref isValid() or
	 *          @ref makeValid() to validate enum value
	 */
	template <typename integer_t>
	LogLevel operator-(const LogLevel& lhs, integer_t rhs)
		requires(std::is_integral_v<integer_t>)
	{
		return static_cast<LogLevel>(static_cast<integer_t>(lhs) - rhs);
	}

	/**
	 * Adds support for addition to @ref LogLevel
	 *
	 * @warning Result is not checked, use @ref isValid() or
	 *          @ref makeValid() to validate enum value
	 */
	template <typename integer_t>
	LogLevel operator+(const LogLevel& lhs, integer_t rhs)
		requires(std::is_integral_v<integer_t>)
	{
		return static_cast<LogLevel>(static_cast<integer_t>(lhs) + rhs);
	}

	/**
	 * @return true if @p level is a valid @ref LogLevel enum, false otherwise
	 */
	constexpr bool isValid(LogLevel level)
	{
		return level >= LogLevel::MinValid && level <= LogLevel::MaxValid;
	}

	/**
	 * Convert a possibly invalid @ref LogLevel into
	 * one that is guaranteed to be valid
	 */
	constexpr LogLevel makeValid(LogLevel level)
	{
		return std::clamp(level, LogLevel::MinValid, LogLevel::MaxValid);
	}

	struct LoggerInternalState;

#ifdef ENGINELOGGING_LIBRARY_IMPLEMENTATION
	// Omit internal data from non-implementation includes

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

#endif

	/**
	 * The type internally responsible for storage of logging prefixes
	 */
	template <typename noptr_class_t> struct CMEP_EXPORT_CLASS logpfx_generator_internal
	{
		CMEP_EXPORT static const char* value;
	};

	/**
	 * Utility struct that generates a logging prefix
	 *
	 * @tparam class_t Type representing the caller (i.e. a method's class),
	 *                 generates an empty string if this is void.
	 */
	template <typename class_t> struct logpfx_generator
	{
		using nocv_remptr_t = std::remove_cv_t<std::remove_pointer_t<class_t>>;
		using prefix_t		= decltype(logpfx_generator_internal<nocv_remptr_t>::value);

		/**
		 * @return The prefix as a string
		 */
		operator prefix_t() const
		{
			return logpfx_generator_internal<nocv_remptr_t>::value;
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

		template <typename class_t> void simpleLog(LogLevel level, const char* format, ...)
		{
			startLog<class_t>(level);

			va_list args;
			va_start(args, format);
			internalLog(format, args);
			va_end(args);

			stopLog();
		}

	private:
		std::unique_ptr<LoggerInternalState> state;

		void internalStartLog(LogLevel level, const char* log_prefix);

		void internalLog(const char* format, va_list args);
	};

	class CMEP_EXPORT_CLASS SupportsLogging
	{
	public:
		using logger_t = std::shared_ptr<Logging::Logger>;

		SupportsLogging() = delete;
		CMEP_EXPORT SupportsLogging(logger_t with_logger) : logger(std::move(with_logger)) {}

		[[nodiscard]] CMEP_EXPORT logger_t getLogger()
		{
			return logger;
		}

	protected:
		logger_t logger;
	};
} // namespace Logging
