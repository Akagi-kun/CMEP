#include <array>
#include <cassert>
#include <cmath>
#include <iomanip>
#include <string>
#include <thread>

#if defined(_MSC_VER)
#include <Windows.h>
#endif

#include "EngineCore.hpp"

#if defined(_MSC_VER)
static void initConsoleWin32()
{
	HANDLE myConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode(myConsole, &dwMode);
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(myConsole, dwMode);
}
#endif

void runEngine()
{
	std::shared_ptr<Logging::Logger> myLogger = std::make_shared<Logging::Logger>();

#if _DEBUG == 1 || defined(DEBUG)
	myLogger->AddOutputHandle(Logging::LogLevel::Debug2, stdout, true);
#else
	myLogger->AddOutputHandle(Logging::LogLevel::Debug1, stdout, true);
#endif

	myLogger->SimpleLog(Logging::LogLevel::Info, "Logger initialized");

	// Initialize engine
	std::unique_ptr<Engine::OpaqueEngine> engine = std::make_unique<Engine::OpaqueEngine>(std::move(myLogger));

	try
	{
		engine->ConfigFile("game/config.json");
	}
	catch (std::exception& e)
	{
		myLogger->SimpleLog(Logging::LogLevel::Exception, "Exception loading config! e.what(): %s", e.what());
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
		myLogger->SimpleLog(Logging::LogLevel::Exception, "Exception caught in rungame! e.what(): %s", e.what());
		throw;
	}
}

int main(int argc, char** argv)
{
	// Enable colored output on Win32
#if defined(_MSC_VER)
	initConsoleWin32();
#endif

	runEngine();

	return 0;
}
