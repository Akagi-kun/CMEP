#include "test_engine.hpp"

#include <string>

#include "Engine.hpp"

Engine::Engine* engine;
const char* windowTitle = "";
const unsigned int windowSizeX = 1200, windowSizeY = 720;

int test_engine()
{
    engine = Engine::initializeEngine(windowTitle, windowSizeX, windowSizeY);
	
	try
	{
		engine->ConfigFile("game/config.json");
		engine->Run();
        return 0;
	}
	catch(std::exception e)
	{
		return 1;
	}
}