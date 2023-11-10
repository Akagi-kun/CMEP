#include <string>
#include <iomanip>
#include <cmath>
#include <cassert>
#include <array>

#if defined(_MSC_VER)
	#include <Windows.h>
#endif

#include "EngineCore.hpp"

Engine::Engine* engine;

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

int main(int argc, char** argv)
{
#if defined(_MSC_VER)
	initStdWin32();
#endif
	
	Engine::EngineConfig config{};
	config.window.sizeX = 1200;
	config.window.sizeY = 720;
	config.window.windowTitle = "My funny game!";
	config.rendering.framerateTarget = 60;

	engine = Engine::initializeEngine(config);
	
	try
	{
		engine->ConfigFile("game/config.json");
		engine->Run();
		
	}
	catch(std::exception e)
	{
		printf("Exception loading config and running engine!");
		exit(-1);
	}
	
	return 0;
}
