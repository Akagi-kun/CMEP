#include "test_engine.hpp"

#include <string>

#include "Engine.hpp"

int test_engine()
{
	Engine::EngineConfig config{};
	config.window.sizeX = 1200;
	config.window.sizeY = 720;
	config.window.title = "My funny game!";
	config.rendering.framerateTarget = 60;

	Engine::Engine* engine = Engine::initializeEngine(config);
	
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
