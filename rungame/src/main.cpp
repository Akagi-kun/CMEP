#include <cassert>
#include <cstdio>
#include <cstring>
#include <exception>

#if defined(_MSC_VER)
#	include <Windows.h>
#endif

#include "Logging/Logging.hpp"

#include "EngineCore.hpp"

#if defined(_MSC_VER)
static void InitConsoleWin32()
{
	HANDLE my_console = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dw_mode	  = 0;
	GetConsoleMode(my_console, &dw_mode);
	dw_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(my_console, dw_mode);
}
#endif

#if _DEBUG == 1 || defined(DEBUG)
#	define DEFAULT_LOG_LEVEL Logging::LogLevel::Debug2
#else
#	define DEFAULT_LOG_LEVEL Logging::LogLevel::Debug1
#endif

// prints the explanatory string of an exception. If the exception is nested,
// recurses to print the explanatory of the exception it holds
static void PrintException(
	std::shared_ptr<Logging::Logger>& with_logger,
	const std::exception& caught_exception,
	int level = 0
)
{
	if (level == 0)
	{
		with_logger->StartLog(Logging::LogLevel::Exception);
		with_logger->Log("Unrolling nested exceptions...");
	}

	with_logger->Log("\n\texception (%i): %s", level, caught_exception.what());

	try
	{
		std::rethrow_if_nested(caught_exception);
	}
	catch (const std::exception& nested_exception)
	{
		PrintException(with_logger, nested_exception, level - 1);
	}
	catch (...)
	{
		throw;
	}

	if (level == 0)
	{
		with_logger->StopLog();
	}
}

static int RunEngine(bool verbose)
{
	std::shared_ptr<Logging::Logger> my_logger = std::make_shared<Logging::Logger>();

	Logging::LogLevel loglevel = verbose ? Logging::LogLevel::Debug3 : DEFAULT_LOG_LEVEL;

	my_logger->AddOutputHandle(loglevel, stdout, true);

	constexpr const char* logfile_name = "latest.log";

	FILE* logfile = nullptr;
#if defined(_MSC_VER)
	fopen_s(&logfile, logfile_name, "w");
#else
	logfile = fopen(logfile_name, "w");
#endif
	if (logfile != nullptr)
	{
		my_logger->AddOutputHandle(Logging::LogLevel::Debug3, logfile, false);
	}
	else
	{
		my_logger->SimpleLog(
			Logging::LogLevel::Warning,
			"Failed opening logfile '%s', will log only to stdout",
			logfile_name
		);
	}

	my_logger->SimpleLog(Logging::LogLevel::Info, "Logger initialized");

	// Initialize engine
	std::unique_ptr<Engine::OpaqueEngine> engine = std::make_unique<Engine::OpaqueEngine>(my_logger);

	// This tests whether exceptions thrown inside EngineCore
	// can be successfully caught in rungame
	try
	{
		engine->ThrowTest();
	}
	catch (std::exception& e)
	{
		if (strcmp(e.what(), "BEBEACAC") == 0)
		{
			my_logger->SimpleLog(Logging::LogLevel::Info, "ABI exception check successful");
		}
		else
		{
			// The exception was caught but is different from expected?
			assert(false && "ABI exception check failed");
		}
	}

	// Initialize engine, load config
	try
	{
		engine->ConfigFile("game/config.json");
		engine->Init();
	}
	catch (std::exception& e)
	{
		my_logger->SimpleLog(Logging::LogLevel::Exception, "Caught exception loading config!");
		PrintException(my_logger, e);
		return 1;
	}

	// Start execution
	try
	{
		engine->Run();
	}
	catch (std::exception& e)
	{
		my_logger->SimpleLog(Logging::LogLevel::Exception, "Caught exception running engine!");
		PrintException(my_logger, e);
		return 2;
	}

	engine.reset();
	my_logger->SimpleLog(Logging::LogLevel::Info, "Bye!");

	return 0;
}

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
	InitConsoleWin32();
#endif

	return RunEngine(verbose);
}
