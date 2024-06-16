#include <cassert>
#include <exception>
#include <string>

#if defined(_MSC_VER)
#	include <Windows.h>
#endif

#include "EngineCore.hpp"

#if defined(_MSC_VER)
static void InitConsoleWin32()
{
	HANDLE my_console = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dw_mode = 0;
	GetConsoleMode(my_console, &dw_mode);
	dw_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(my_console, dw_mode);
}
#endif

#if _DEBUG == 1 || defined(DEBUG)
#	define DEFAULT_LOG_LEVEL Logging::LogLevel::Debug3
#else
#	define DEFAULT_LOG_LEVEL Logging::LogLevel::Debug1
#endif

static void RunEngine()
{
	std::shared_ptr<Logging::Logger> my_logger = std::make_shared<Logging::Logger>();

	my_logger->AddOutputHandle(DEFAULT_LOG_LEVEL, stdout, true);

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
}

int main(int argc, char** argv)
{
	// Command-line arguments unused
	(void)argc;
	(void)argv;

#if defined(_MSC_VER)
	// Enable colored output on Win32
	InitConsoleWin32();
#endif

	RunEngine();

	return 0;
}
