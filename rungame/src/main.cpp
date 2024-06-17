#include <cassert>
#include <cstdio>
#include <exception>
#include <fstream>
#include <string>

#if defined(_MSC_VER)
#	include <Windows.h>
#endif

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

static void RunEngine(bool verbose)
{
	std::shared_ptr<Logging::Logger> my_logger = std::make_shared<Logging::Logger>();

	Logging::LogLevel loglevel = DEFAULT_LOG_LEVEL;
	if (verbose)
	{
		loglevel = Logging::LogLevel::Debug3;
	}

	FILE* logfile = fopen("latest.log", "w");

	my_logger->AddOutputHandle(loglevel, stdout, true);
	my_logger->AddOutputHandle(Logging::LogLevel::Debug3, logfile, false);

	my_logger->SimpleLog(Logging::LogLevel::Info, "Logger initialized");

	// Initialize engine
	std::unique_ptr<Engine::OpaqueEngine> engine = std::make_unique<Engine::OpaqueEngine>(my_logger);

	// Initialize engine, load config
	try
	{
		engine->ConfigFile("game/config.json");
		engine->Init();
	}
	catch (std::exception& e)
	{
		my_logger->SimpleLog(Logging::LogLevel::Exception, "Exception loading config! e.what(): %s", e.what());
		throw;
	}

	// Start execution
	try
	{
		engine->Run();
	}
	catch (std::exception& e)
	{
		my_logger->SimpleLog(Logging::LogLevel::Exception, "Exception running engine! e.what(): %s", e.what());
		throw;
	}

	fclose(logfile);
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

	RunEngine(verbose);

	return 0;
}
