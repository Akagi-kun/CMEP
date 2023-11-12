#include <string>
#include <iomanip>
#include <cmath>
#include <cassert>
#include <array>
#include <thread>

#if defined(_MSC_VER)
	#include <Windows.h>
#endif

#include "EngineCore.hpp"

#if defined(_MSC_VER)
static void initStdWin32()
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
	Engine::Engine* engine = nullptr;

	Engine::EngineConfig config{};
	config.window.sizeX = 1200;
	config.window.sizeY = 720;
	config.window.title = "My funny game!";
	config.rendering.framerateTarget = 60;
	config.lookup.scenes = "game/scenes";
	config.defaultScene = "floppygame";

	std::shared_ptr<Logging::Logger> myLogger = std::make_shared<Logging::Logger>();
#if _DEBUG == 1 || defined(DEBUG)
	myLogger->AddOutputHandle(Logging::LogLevel::Debug3, stdout, true);
#else
	myLogger->AddOutputHandle(Logging::LogLevel::Debug1, stdout, true);
#endif

		// Initialize engine
	engine = new Engine::Engine(myLogger, config);
	engine->Init();

	try
	{
		engine->ConfigFile("game/config.json");
		
	}
	catch(std::exception e)
	{
		printf("Exception loading config! %s\n", e.what());
		throw e;
	}
	
	engine->Run();

	delete engine;
}

int main(int argc, char** argv)
{
#if defined(_MSC_VER)
	initStdWin32();
#endif
	
	runEngine();
	
	return 0;
}
