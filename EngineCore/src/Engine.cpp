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
			}
		}
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
			this->logger->SimpleLog(Logging::LogLevel::Exception, "Error parsing config json '%s', what: %s", this->config_path.c_str(), e.what());
			exit(1);
		}

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

			this->logger->SimpleLog(Logging::LogLevel::Debug3, "Event handler for type: %s", static_cast<std::string>(eventHandler["type"]).c_str());
			std::shared_ptr<Scripting::LuaScript> event_handler = asset_manager->GetLuaScript(eventHandler["file"]);
			
			if(event_handler == nullptr)
			{
				asset_manager->AddLuaScript(eventHandler["file"], eventHandler["file"]);
				event_handler = asset_manager->GetLuaScript(eventHandler["file"]);
			}
			
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
		std::shared_ptr<GlobalSceneManager> scene_manager = std::make_shared<GlobalSceneManager>(global_scene_manager);
		for (auto& [name, ptr] : (scene_manager->GetAllObjects()))
		{
			try
			{
				ptr->renderer->Render(commandBuffer, currentFrame);
			}
			catch (const std::exception& e)
			{
				ptr->renderer->logger->SimpleLog(Logging::LogLevel::Exception, "Caught exception while rendering object %s: %s", name.c_str(), e.what());
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
			event.raisedFrom = global_engine;
			global_engine->FireEvent(event);
		}
		else if(action == GLFW_RELEASE)
		{
			EventHandling::Event event = EventHandling::Event(EventHandling::EventType::ON_KEYUP);
			event.keycode = key;
			event.deltaTime = global_engine->GetLastDeltaTime();
			event.raisedFrom = global_engine;
			global_engine->FireEvent(event);
		}
	}

	void Engine::engineLoop()
	{		
		Object* object = new Object();
		object->renderer = new Rendering::AxisRenderer();
		object->Translate(glm::vec3(0, 0, 0));
		object->Scale(glm::vec3(1, 1, 1));
		object->Rotate(glm::vec3(0, 0, 0));
		object->ScreenSizeInform(this->windowX, this->windowY);
		((Rendering::AxisRenderer*)object->renderer)->UpdateMesh();
		global_scene_manager->AddObject("_axis", object);
		
		// Pre-make ON_UPDATE event so we don't have to create it over and over again in hot loop
		EventHandling::Event premadeOnUpdateEvent = EventHandling::Event(EventHandling::EventType::ON_UPDATE);
		premadeOnUpdateEvent.raisedFrom = this;
		
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

			// Update deltaTime of premade ON_UPDATE event and fire it
			premadeOnUpdateEvent.deltaTime = deltaTime;
			if(this->FireEvent(premadeOnUpdateEvent) != 0)
			{
				break;
			}

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

	Engine::Engine(std::shared_ptr<Logging::Logger> logger, std::string windowTitle, const unsigned windowX, const unsigned windowY) noexcept : logger(logger), windowTitle(windowTitle), windowX(windowX), windowY(windowY), framerateTarget(30) {}

	Engine::~Engine() noexcept
	{
		delete this->asset_manager;

		delete this->script_executor;

		this->rendering_engine->cleanup();

		delete this->rendering_engine;
	}

	void Engine::SetFramerateTarget(unsigned framerate) noexcept
	{
		this->framerateTarget = framerate;
	}

	void Engine::Init()
	{
		this->logger->MapCurrentThreadToName("engine");

		// Engine info printout
#if defined(_MSC_VER)
		this->logger->SimpleLog(Logging::LogLevel::Info, 
			"Engine info:\n////\nRunning CMEP EngineCore %s %s build, configured %s\nCompiled by MSVC compiler version: %u.%u\n////\n", 
			__TIME__, __DATE__, _DEBUG ? "DEBUG" : "RELEASE", _MSC_FULL_VER, _MSC_BUILD
		);
#elif defined(__GNUC__)
		this->logger->SimpleLog(Logging::LogLevel::Info, 
			"Engine info:\n////\nRunning CMEP EngineCore %s %s build\nCompiled by GCC compiler version: %u.%u.%u\n////\n", 
			__TIME__, __DATE__, __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__
		);
#else
		this->logger->SimpleLog(Logging::LogLevel::Info, 
			"Engine info:\n////\nRunning CMEP EngineCore %s %s build\nCompiled by unknown compiler\n////\n", 
			__TIME__, __DATE__
		);
#endif

		//this->window = new Rendering::Window(this->windowTitle, this->windowX, this->windowY);
		this->script_executor = new Scripting::LuaScriptExecutor();
		this->script_executor->UpdateHeldLogger(this->logger);

		this->asset_manager = new AssetManager();
		this->asset_manager->logger = this->logger;
		this->asset_manager->lua_executor = this->script_executor;
		//this->asset_manager->UpdateEngine(this);

		this->scene_manager = std::make_shared<GlobalSceneManager>();
		this->scene_manager->logger = this->logger;

		this->rendering_engine = new Rendering::VulkanRenderingEngine();
		this->rendering_engine->logger = this->logger;
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
			this->logger->SimpleLog(Logging::LogLevel::Exception, "Failed handling config! e.what(): %s", e.what());
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
		this->logger->SimpleLog(Logging::LogLevel::Debug1, "Handling ON_INIT took %.3lf ms total and returned %i", total, onInitEventRet);
		
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

	void Engine::RegisterLuaEventHandler(EventHandling::EventType event_type, std::shared_ptr<Scripting::LuaScript> script, std::string function)
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

	std::weak_ptr<GlobalSceneManager> Engine::GetSceneManager() noexcept
	{
		std::weak_ptr<GlobalSceneManager> weakSceneManager = this->scene_manager;
		
		return weakSceneManager;
	}

	Engine* initializeEngine(EngineConfig config)
	{
		// Set up loggre
		std::shared_ptr<Logging::Logger> myLogger = std::make_shared<Logging::Logger>();
#if _DEBUG == 1 || defined(DEBUG)
		myLogger->AddOutputHandle(Logging::LogLevel::Debug3, stdout, true);
#else
		myLogger->AddOutputHandle(Logging::LogLevel::Debug1, stdout, true);
#endif

		// Set up global scene
		global_scene_manager = new GlobalSceneManager();
		global_scene_manager->logger = myLogger;
		
		// Initialize engine
		global_engine = new Engine(myLogger, config.window.title, config.window.sizeX, config.window.sizeY);
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