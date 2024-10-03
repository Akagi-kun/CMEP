#include "EngineCore.hpp"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <memory>

#if defined(SEMANTICS_MSVC)
#	include <Windows.h>
#endif

#include "Logging/Logging.hpp"

namespace
{
#if defined(SEMANTICS_MSVC)
	void initConsoleWin32()
	{
		HANDLE my_console = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD  dw_mode	  = 0;
		GetConsoleMode(my_console, &dw_mode);
		dw_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(my_console, dw_mode);
	}
#endif

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
	 * This likely isn't necessary now, but used to be a problem in the past on Windows
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
			assert(false && "exception check failed");
			return false;
		}
	}

	int runEngine(bool verbose)
	{
		std::shared_ptr<Logging::Logger> logger = std::make_shared<Logging::Logger>();

		const Logging::LogLevel stdout_loglevel = verbose ? VERBOSE_LOG_LEVEL
														  : DEFAULT_LOG_LEVEL;

		logger->addOutputHandle(stdout_loglevel, stdout, true);

		constexpr const char* logfile_name = "latest.log";

		FILE* logfile = nullptr;
#if defined(SEMANTICS_MSVC)
		fopen_s(&logfile, logfile_name, "w");
#else
		logfile = fopen(logfile_name, "w");
#endif
		if (logfile != nullptr)
		{
			logger->addOutputHandle(VERBOSE_LOG_LEVEL, logfile, false);
		}
		else
		{
			logger->simpleLog<void>(
				Logging::LogLevel::Warning,
				"Failed opening logfile '%s', will log only to stdout",
				logfile_name
			);
		}

		logger->simpleLog<void>(Logging::LogLevel::Info, "Logger initialized");

		// Initialize engine
		std::unique_ptr<Engine::OpaqueEngine> engine =
			std::make_unique<Engine::OpaqueEngine>(logger);

		// This tests whether exceptions thrown inside EngineCore
		// can be successfully caught in rungame
		if (!checkExceptions(engine))
		{
			logger->simpleLog<void>(
				Logging::LogLevel::Error,
				"checkABI returned false! aborting"
			);
			std::abort();
		}

		logger->simpleLog<void>(Logging::LogLevel::Info, "exception check successful");

		// Initialize engine, load config
		try
		{
			engine->configFile("game/config.json");
		}
		catch (std::exception& e)
		{
			logger->simpleLog<void>(
				Logging::LogLevel::Exception,
				"Caught exception loading config!\n%s",
				Engine::unrollExceptions(e).c_str()
			);

			return 1;
		}

		// Start execution
		try
		{
			engine->run();
		}
		catch (std::exception& e)
		{
			logger->simpleLog<void>(
				Logging::LogLevel::Exception,
				"Caught exception running engine!\n%s",
				Engine::unrollExceptions(e).c_str()
			);
			return 2;
		}

		engine.reset();
		logger->simpleLog<void>(Logging::LogLevel::Info, "Bye!");

		return 0;
	}
} // namespace

int main(int argc, char** argv)
{
	bool verbose = false;

	if (argc > 1)
	{
		if (strcmp(argv[1], "-v") == 0) { verbose = true; }
	}

#if defined(SEMANTICS_MSVC)
	// Enable colored output on Win32
	initConsoleWin32();
#endif

	return runEngine(verbose);
}
