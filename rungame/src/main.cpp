#include <cassert>
#include <exception>
#include <ostream>
#include <string>
#include <thread>

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

static void RunEngine()
{
	std::shared_ptr<Logging::Logger> my_logger = std::make_shared<Logging::Logger>();

#if _DEBUG == 1 || defined(DEBUG)
	my_logger->AddOutputHandle(Logging::LogLevel::Debug2, stdout, true);
#else
	myLogger->AddOutputHandle(Logging::LogLevel::Debug1, stdout, true);
#endif

	my_logger->SimpleLog(Logging::LogLevel::Info, "Logger initialized");

	// Initialize engine
	std::unique_ptr<Engine::OpaqueEngine> engine = std::make_unique<Engine::OpaqueEngine>(my_logger);

	try
	{
		engine->ConfigFile("game/config.json");
	}
	catch (std::exception& e)
	{
		my_logger->SimpleLog(Logging::LogLevel::Exception, "Exception loading config! e.what(): %s", e.what());
		throw;
	}

	// Start execution
	try
	{
		engine->Init();
		engine->Run();
	}
	catch (std::exception& e)
	{
		my_logger->SimpleLog(Logging::LogLevel::Exception, "Exception caught in rungame! e.what(): %s", e.what());
		throw;
	}
}

int main(int argc, char** argv)
{
	// Enable colored output on Win32
#if defined(_MSC_VER)
	InitConsoleWin32();
#endif

	RunEngine();

	return 0;
}
