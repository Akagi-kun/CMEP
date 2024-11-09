#include "Logging/Logging.hpp"

#include "EngineCore.hpp"
#include "win32_console.hpp"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <memory>
#include <string>

namespace
{
	struct StartupConfig
	{
		int verbosity_level = 0;
	};

// Debug builds should by default log more than release builds
#if _DEBUG == 1 || defined(DEBUG)
// Debug
#	define DEFAULT_LOG_LEVEL Logging::LogLevel::Debug
#	define VERBOSE_LOG_LEVEL Logging::LogLevel::VerboseDebug
#else
// Release
#	define DEFAULT_LOG_LEVEL Logging::LogLevel::Info
#	define VERBOSE_LOG_LEVEL Logging::LogLevel::Debug
#endif

	/**
	 * @brief Checks whether exceptions can be caught when thrown by the engine
	 *
	 * This likely isn't necessary now, but used to be a problem on Windows in the past
	 * when exceptions weren't logged because they couldn't be caught
	 * across the shared-library/executable ABI
	 */
	bool checkExceptions(std::unique_ptr<Engine::OpaqueEngine>& engine)
	{
		try
		{
			engine->throwTest();
		}
		catch (std::exception& e)
		{
			if (strcmp(e.what(), "BEBEACAC") == 0) { return true; }

			// The exception was caught but is different from expected?
			assert(false && "Exception check failed");
			return false;
		}
	}

	void onEngineExcept(
		const std::shared_ptr<Logging::Logger>& logger,
		const std::exception&					exception,
		const std::string&						message
	)
	{
		logger->logSingle<void>(
			Logging::LogLevel::Exception,
			"{}\n{}",
			message.c_str(),
			Engine::Base::unrollExceptions(exception).c_str()
		);
	}

	int runEngine(const StartupConfig& config)
	{
		std::shared_ptr<Logging::Logger> logger = std::make_shared<Logging::Logger>();

		const Logging::LogLevel stdout_loglevel =
			Logging::makeValid(DEFAULT_LOG_LEVEL - config.verbosity_level);

		logger->addOutputHandle(stdout_loglevel, stdout, true);

		const char* logfile_name = "latest.log";

		FILE* logfile = nullptr;
#if defined(SEMANTICS_MSVC)
		fopen_s(&logfile, logfile_name, "w");
#else
		logfile = fopen(logfile_name, "w");
#endif
		if (logfile != nullptr)
		{
			logger->addOutputHandle(std::min(VERBOSE_LOG_LEVEL, stdout_loglevel), logfile);
		}
		else
		{
			logger->logSingle<void>(
				Logging::LogLevel::Warning,
				"Failed opening logfile '{}', will log only to stdout",
				logfile_name
			);
		}

		logger->logSingle<void>(
			Logging::LogLevel::Info,
			"Logger initialized (stdout loglevel is {})",
			static_cast<std::underlying_type_t<Logging::LogLevel>>(stdout_loglevel)
		);

		// Initialize engine
		std::unique_ptr<Engine::OpaqueEngine> engine = std::make_unique<Engine::OpaqueEngine>(logger
		);

		// This tests whether exceptions thrown inside EngineCore
		// can be successfully caught in rungame
		if (!checkExceptions(engine))
		{
			logger->logSingle<void>(Logging::LogLevel::Error, "checkABI returned false! aborting");
			std::abort();
		}

		logger->logSingle<void>(Logging::LogLevel::Info, "Exception check successful");

		// Initialize engine, load config
		try
		{
			engine->configFile("game/config.json");
		}
		catch (const std::exception& e)
		{
			onEngineExcept(logger, e, "Caught exception loading config!");

			return 1;
		}

		// Start execution
		try
		{
			engine->run();
		}
		catch (const std::exception& e)
		{
			onEngineExcept(logger, e, "Caught exception running engine!");

			return 2;
		}

		engine.reset();
		logger->logSingle<void>(Logging::LogLevel::Info, "Bye!");

		return 0;
	}

	/**
	 * @brief Parses a single argument string
	 *
	 * @param[out] config Config to alter using the arguments
	 * @param[in] arg_str Argument received from @p argv
	 */
	void parseArg(StartupConfig& config, char* arg_str)
	{
		if (strcmp(arg_str, "-v") == 0) { config.verbosity_level = 1; }
		else if (strcmp(arg_str, "-vv") == 0) { config.verbosity_level = 2; }
	}
} // namespace

int main(int argc, char** argv)
{
#if defined(SEMANTICS_MSVC)
	// Enable colored output on Win32
	initConsoleWin32();
#endif

	StartupConfig config;

	for (int arg_idx = 1; arg_idx < argc; arg_idx++)
	{
		parseArg(config, argv[arg_idx]);
	}

	return runEngine(config);
}
