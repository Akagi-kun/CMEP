#include <assert.h>
#include <fstream>
#include <memory>
#include <thread>
#include <chrono>

#include "nlohmann-json/single_include/nlohmann/json.hpp"

#include "glm/glm.hpp"

#include "Rendering/AxisRenderer.hpp"

#include "Scripting/LuaScriptExecutor.hpp"
#include "Scripting/LuaScript.hpp"

#include "ObjectFactory.hpp"
#include "AssetManager.hpp"
#include "Engine.hpp"
#include "Object.hpp"

#if defined(_MSC_VER)
#include <Windows.h>
#endif

#if defined(_MSC_VER)
	#ifndef _DEBUG
		#define _DEBUG 0
	#endif
#endif

namespace Engine
{
	bool EngineIsWindowInFocus = false;
	bool EngineIsWindowInContent = false;
	double EngineMouseXPos = 0.0;
	double EngineMouseYPos = 0.0;

	// Utility sleep function
	void Engine::spinSleep(double seconds)
	{
		static double estimate = 5e-3;
		static double mean = 5e-3;
		static double m2 = 0;
		static int64_t count = 1;

		while (seconds > estimate) {
			const auto start = std::chrono::steady_clock::now();
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
			const auto end = std::chrono::steady_clock::now();

			const double observed = (end - start).count() / 1e9;
			seconds -= observed;

			count++;
			const double delta = observed - mean;
			mean += delta / count;
			m2 += delta * (observed - mean);
			const double stddev = sqrt(m2 / (count - 1));
			estimate = mean + stddev;
		}

		// spin lock
		const auto start = std::chrono::steady_clock::now();
		while ((std::chrono::steady_clock::now() - start).count() / 1e9 < seconds) {};
	}

	void Engine::handleInput(const double deltaTime) noexcept
	{
		Rendering::GLFWwindowData windowdata = this->rendering_engine->GetWindow();

		double xpos = 0.0, ypos = 0.0;
		static double lastx = (windowdata.windowX / 2.0), lasty = (windowdata.windowY / 2.0);

		//glfwGetCursorPos(windowdata.window, &xpos, &ypos);

		//if (glfwGetWindowAttrib(windowdata.window, GLFW_FOCUSED))
		if (EngineIsWindowInFocus && EngineIsWindowInContent)
		{
			if ((EngineMouseXPos - lastx) != 0.0 || (EngineMouseYPos - lasty) != 0.0)
			{
				EventHandling::Event event = EventHandling::Event(EventHandling::EventType::ON_MOUSEMOVED);
				event.mouse.x = EngineMouseXPos - lastx;
				event.mouse.y = EngineMouseYPos - lasty;
				event.deltaTime = deltaTime;
				this->FireEvent(event);

				lastx = EngineMouseXPos;
				lasty = EngineMouseYPos;

				//glfwSetCursorPos(this->rendering_engine->GetWindow().window, this->windowX / 2, this->windowY / 2);
			}
		}

		// Handle keyboard + mouse buttons
		// for (uint16_t i = 1; i < 256; i++)
		// {
		// 	if (glfwGetKey(windowdata.window, i))
		// 	{
		// 		EventHandling::Event event = EventHandling::Event(EventHandling::EventType::ON_KEYDOWN);
		// 		event.keycode = i;
		// 		event.deltaTime = deltaTime;
		// 		this->FireEvent(event);
		// 	}
		// }
	}

	void Engine::HandleConfig()
	{
		std::ifstream file(this->config_path);
		
		nlohmann::json data;
		try
		{
			data = nlohmann::json::parse(file);
		}
		catch(std::exception& e)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Exception, "Error parsing config json '%s', what: %s", this->config_path.c_str(), e.what());
			exit(1);
		}

		Scripting::LuaScript* event_handler = nullptr;
		EventHandling::EventType eventType = EventHandling::EventType::EVENT_UNDEFINED;

		for(auto& eventHandler : data["eventHandlers"])
		{
			if(eventHandler["type"] == std::string("onInit"))
			{
				eventType = EventHandling::EventType::ON_INIT;
			}
			else if(eventHandler["type"] == std::string("onMouseMoved"))
			{
				eventType = EventHandling::EventType::ON_MOUSEMOVED;
			}
			else if(eventHandler["type"] == std::string("onKeyDown"))
			{
				eventType = EventHandling::EventType::ON_KEYDOWN;
			}
			else if(eventHandler["type"] == std::string("onKeyUp"))
			{
				eventType = EventHandling::EventType::ON_KEYUP;
			}
			else if(eventHandler["type"] == std::string("onUpdate"))
			{
				eventType = EventHandling::EventType::ON_UPDATE;
			}

			assert(eventType != EventHandling::EventType::EVENT_UNDEFINED);

			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug3, "Event handler for type: %s", static_cast<std::string>(eventHandler["type"]).c_str());
			asset_manager->AddLuaScript(eventHandler["file"], eventHandler["file"]);
			event_handler = asset_manager->GetLuaScript(eventHandler["file"]);
			global_engine->RegisterLuaEventHandler(eventType, event_handler, eventHandler["function"]);
		}

		std::string setting;
		setting = "window.title";
		this->windowTitle = data[setting];

		setting = "window.size";
		this->windowX = data[setting]["x"]; this->windowY = data[setting]["y"];
	}

	void Engine::RenderCallback(VkCommandBuffer commandBuffer, uint32_t currentFrame)
	{
		for (auto& [name, ptr] : *global_scene_manager->GetAllObjects())
		{
			try
			{
				ptr->renderer->Render(commandBuffer, currentFrame);
			}
			catch (const std::exception& e)
			{
				Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Exception, "Caught exception while rendering object %s: %s", name.c_str(), e.what());
				exit(1);
			}
		}
	}
	
	void Engine::OnWindowFocusCallback(GLFWwindow* window, int focused)
	{
		if(focused)
		{
			EngineIsWindowInFocus = true;
		}
		else
		{
			EngineIsWindowInFocus = false;
		}
	}

	void Engine::CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
	{
		if(EngineIsWindowInFocus)
		{
			EngineMouseXPos = xpos;
			EngineMouseYPos = ypos;
		}	
		else
		{
			EngineMouseXPos = 0.0;
			EngineMouseYPos = 0.0;
		}
	}

	void Engine::CursorEnterLeaveCallback(GLFWwindow* window, int entered)
	{
		if(EngineIsWindowInFocus)
		{
			EngineIsWindowInContent = true;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else
		{
			EngineIsWindowInContent = false;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
	
	void Engine::OnKeyEventCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			EventHandling::Event event = EventHandling::Event(EventHandling::EventType::ON_KEYDOWN);
			event.keycode = key;
			event.deltaTime = global_engine->GetLastDeltaTime();
			global_engine->FireEvent(event);
		}
		else if(action == GLFW_RELEASE)
		{
			EventHandling::Event event = EventHandling::Event(EventHandling::EventType::ON_KEYUP);
			event.keycode = key;
			event.deltaTime = global_engine->GetLastDeltaTime();
			global_engine->FireEvent(event);
		}
	}

	void Engine::engineLoop()
	{
		Logging::GlobalLogger->MapCurrentThreadToName("game");
		
		//Object* object = new Object();
		//object->renderer = new Rendering::AxisRenderer();
		//object->Translate(glm::vec3(0, 0, 0));
		//object->Scale(glm::vec3(1, 1, 1));
		//object->Rotate(glm::vec3(0, 0, 0));
		//object->ScreenSizeInform(this->windowX, this->windowY);
		//((Rendering::AxisRenderer*)object->renderer)->UpdateMesh();
		//global_scene_manager->AddObject("_axis", object);
		
		// Object* sprite_test = ObjectFactory::CreateSpriteObject(0.0, 0.0, 0.5, 0.5, this->asset_manager->GetTexture("game/fonts/myfont/myfont_0.png"));
		// global_scene_manager->AddObject("testpng", sprite_test);


		//glEnable(GL_MULTISAMPLE);
		//glEnable(GL_DEPTH_TEST);
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//glViewport(0, 0, this->windowX, this->windowY);

		// Pre-make ON_UPDATE event so we don't have to create it over and over again in hot loop
		EventHandling::Event premadeOnUpdateEvent = EventHandling::Event(EventHandling::EventType::ON_UPDATE);
		
		glfwShowWindow(this->rendering_engine->GetWindow().window);

		int counter = 0;
		auto prevClock = std::chrono::steady_clock::now();
		// hot loop
		while (!glfwWindowShouldClose(this->rendering_engine->GetWindow().window))
		{
			const auto nextClock = std::chrono::steady_clock::now();
			const double deltaTime = (nextClock - prevClock).count() / 1e9;
			if (counter == this->framerateTarget)
			{
				// For debugging, use onscreen counter if possible
				//printf("current frame time: %.2lf ms (%.1lf fps)\n", deltaTime * 1e3, (1 / (deltaTime)));
				counter = 0;
			}
			this->lastDeltaTime = deltaTime;

			// Handle input
			//this->handleInput(deltaTime);

			// Update deltaTime of premade ON_UPDATE event and fire it
			premadeOnUpdateEvent.deltaTime = deltaTime;
			if(this->FireEvent(premadeOnUpdateEvent) != 0)
			{
				break;
			}

			// Clear screen
			//glClearColor(0.1, 0.13, 0.2, 1.0);
			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Render every object
			/*
			for (auto& [name, ptr] : *global_scene_manager->GetAllObjects())
			{
				try
				{
					ptr->renderer->Render();
					//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				}
				catch (const std::exception& e)
				{
					Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Exception, "Caught exception while rendering object %s: %s", name.c_str(), e.what());
					exit(1);
				}
			}
			*/

			// Render
			this->rendering_engine->drawFrame();
			
			glfwSwapBuffers(this->rendering_engine->GetWindow().window);
			glfwPollEvents();
			
			// spin sleep if framerate locked
			if(this->framerateTarget == 0)
			{
				this->framerateTarget = 5000;
			}

			const auto frameClock = std::chrono::steady_clock::now();
			const double sleepSecs = 1.0 / this->framerateTarget - (frameClock - nextClock).count() / 1e9;
			if (sleepSecs > 0 && framerateTarget != 0)
			{
				spinSleep(sleepSecs);
			}

			prevClock = nextClock;
			counter++;
		}
	}

	int Engine::FireEvent(EventHandling::Event& event)
	{
		int sum = 0;
		for (auto handler : this->event_handlers)
		{
			if (handler.first == event.event_type)
			{
				sum += handler.second(event);
			}
		}

		for (auto handler : this->lua_event_handlers)
		{
			if (std::get<0>(handler) == event.event_type)
			{
				sum += this->script_executor->CallIntoScript(Scripting::ExecuteType::EventHandler, std::get<1>(handler), std::get<2>(handler), &event);
			}
		}
		return sum;
	}

	double Engine::GetLastDeltaTime()
	{
		return this->lastDeltaTime;
	}

	Engine::Engine(const char* windowTitle, const unsigned windowX, const unsigned windowY) noexcept : windowTitle(windowTitle), windowX(windowX), windowY(windowY), framerateTarget(30) {}

	Engine::~Engine() noexcept
	{
		delete this->asset_manager;

		this->rendering_engine->cleanup();

		delete this->script_executor;

		delete this->rendering_engine;
	}

	void Engine::SetFramerateTarget(unsigned framerate) noexcept
	{
		this->framerateTarget = framerate;
	}

	void Engine::Init()
	{
		Logging::GlobalLogger->MapCurrentThreadToName("engine");

		// Engine info printout
#if defined(_MSC_VER)
		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Info, 
			"Engine info:\n////\nRunning CMEP EngineCore %s %s build, configured %s\nCompiled by MSVC compiler version: %u.%u\n////\n", 
			__TIME__, __DATE__, _DEBUG ? "DEBUG" : "RELEASE", _MSC_FULL_VER, _MSC_BUILD
		);
#elif defined(__GNUC__)
		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Info, 
			"Engine info:\n////\nRunning CMEP EngineCore %s %s build\nCompiled by GCC compiler version: %u.%u.%u\n////\n", 
			__TIME__, __DATE__, __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__
		);
#else
		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Info, 
			"Engine info:\n////\nRunning CMEP EngineCore %s %s build\nCompiled by unknown compiler\n////\n", 
			__TIME__, __DATE__
		);
#endif

		//this->window = new Rendering::Window(this->windowTitle, this->windowX, this->windowY);
		this->asset_manager = new AssetManager();
		this->script_executor = new Scripting::LuaScriptExecutor();
		this->rendering_engine = new Rendering::VulkanRenderingEngine();
	}

	void Engine::Run()
	{
		auto start = std::chrono::steady_clock::now();

		try
		{
			this->HandleConfig();
		}
		catch(std::exception e)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Exception, "Failed handling config! e.what(): %s", e.what());
			exit(1);
		}

		this->rendering_engine->init(this->windowX, this->windowY, this->windowTitle);
		this->rendering_engine->SetRenderCallback(this->RenderCallback);

		Rendering::GLFWwindowData windowdata = this->rendering_engine->GetWindow();
		glfwSetWindowFocusCallback(windowdata.window, Engine::OnWindowFocusCallback);
		glfwSetCursorPosCallback(windowdata.window, Engine::CursorPositionCallback);
		glfwSetCursorEnterCallback(windowdata.window, Engine::CursorEnterLeaveCallback);
		glfwSetKeyCallback(windowdata.window, Engine::OnKeyEventCallback);
		
		// Fire ON_INIT event
		EventHandling::Event onInitEvent = EventHandling::Event(EventHandling::EventType::ON_INIT);
		int onInitEventRet = this->FireEvent(onInitEvent);

		// Measure and log ON_INIT time
		double total = (std::chrono::steady_clock::now() - start).count() / 1e6;
		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug1, "Handling ON_INIT took %.3lf ms total and returned %i", total, onInitEventRet);
		
		if(onInitEventRet != 0)
		{
			return;
		}

		this->engineLoop();
	}

	void Engine::ConfigFile(std::string path)
	{
		this->config_path = std::move(path);
	}

	void Engine::RegisterEventHandler(EventHandling::EventType event_type, std::function<int(EventHandling::Event&)> function)
	{
		this->event_handlers.push_back(std::make_pair(event_type, function));
	}

	void Engine::RegisterLuaEventHandler(EventHandling::EventType event_type, Scripting::LuaScript* script, std::string function)
	{
		this->lua_event_handlers.push_back(std::make_tuple(event_type, script, function));
	}

	AssetManager* Engine::GetAssetManager() noexcept
	{
		return this->asset_manager;
	}

	Rendering::VulkanRenderingEngine* Engine::GetRenderingEngine() noexcept
	{
		return this->rendering_engine;
	}

	Engine* initializeEngine(const char* windowTitle, const unsigned windowX, const unsigned windowY)
	{
		// Set up loggre
		Logging::GlobalLogger = std::make_unique<Logging::Logger>();
#if _DEBUG == 1 || defined(DEBUG)
		Logging::GlobalLogger->AddOutputHandle(Logging::LogLevel::Debug3, stdout, true);
#else
		Logging::GlobalLogger->AddOutputHandle(Logging::LogLevel::Debug1, stdout, true);
#endif

		// Set up global scene
		global_scene_manager = new GlobalSceneManager();
		
		// Initialize engine
		global_engine = new Engine(windowTitle, windowX, windowY);
		global_engine->Init();

		return global_engine;
	}

	int deinitializeEngine()
	{
		delete global_scene_manager;
		delete global_engine;

		return 0;
	}

	CMEP_EXPORT Engine* global_engine;
}