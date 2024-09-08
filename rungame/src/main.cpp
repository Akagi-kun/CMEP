#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <memory>

#if defined(_MSC_VER)
#	include <Windows.h>
#endif

#include "Logging/Logging.hpp"

#include "EngineCore.hpp"

namespace
{
#if defined(_MSC_VER)
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

	int runEngine(bool verbose)
	{
		std::shared_ptr<Logging::Logger> my_logger = std::make_shared<Logging::Logger>();

		const Logging::LogLevel stdout_loglevel = verbose ? VERBOSE_LOG_LEVEL : DEFAULT_LOG_LEVEL;

		my_logger->addOutputHandle(stdout_loglevel, stdout, true);

		constexpr const char* logfile_name = "latest.log";

		FILE* logfile = nullptr;
#if defined(_MSC_VER)
		fopen_s(&logfile, logfile_name, "w");
#else
		logfile = fopen(logfile_name, "w");
#endif
		if (logfile != nullptr)
		{
			my_logger->addOutputHandle(VERBOSE_LOG_LEVEL, logfile, false);
		}
		else
		{
			my_logger->simpleLog<void>(
				Logging::LogLevel::Warning,
				"Failed opening logfile '%s', will log only to stdout",
				logfile_name
			);
		}

		my_logger->simpleLog<void>(Logging::LogLevel::Info, "Logger initialized");

		// Initialize engine
		std::unique_ptr<Engine::OpaqueEngine> engine = std::make_unique<Engine::OpaqueEngine>(
			my_logger
		);

		// This tests whether exceptions thrown inside EngineCore
		// can be successfully caught in rungame
		try
		{
			engine->throwTest();
		}
		catch (std::exception& e)
		{
			if (strcmp(e.what(), "BEBEACAC") == 0)
			{
				my_logger->simpleLog<void>(
					Logging::LogLevel::Info,
					"ABI exception check successful"
				);
			}
			else
			{
				// The exception was caught but is different from expected?
				assert(false && "ABI exception check failed");
				std::abort();
			}
		}

		// Initialize engine, load config
		try
		{
			engine->configFile("game/config.json");
		}
		catch (std::exception& e)
		{
			my_logger->simpleLog<void>(
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
			my_logger->simpleLog<void>(
				Logging::LogLevel::Exception,
				"Caught exception running engine!\n%s",
				Engine::unrollExceptions(e).c_str()
			);
			return 2;
		}

		engine.reset();
		my_logger->simpleLog<void>(Logging::LogLevel::Info, "Bye!");

		return 0;
	}
} // namespace

int main(int argc, char** argv)
{
	bool verbose = false;

	if (argc > 1)
	{
		if (strcmp(argv[1], "-v") == 0)
		{
			verbose = true;
		}
	}

#if defined(_MSC_VER)
	// Enable colored output on Win32
	initConsoleWin32();
#endif

	return runEngine(verbose);
}
