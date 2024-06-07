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

#include "Factories/ObjectFactory.hpp"
#include "AssetManager.hpp"
#include "Engine.hpp"
#include "Object.hpp"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_ENGINE
#include "Logging/LoggingPrefix.hpp"

//#if defined(_MSC_VER)
//	#ifndef _DEBUG
//		#define _DEBUG 0
//	#endif
//#endif

#ifdef DEBUG
	#define IS_DEBUG 1
#else
	#define IS_DEBUG 0
#endif

#if IS_DEBUG == 1
	#define BUILDCONFIG "DEBUG"
#else
	#define BUILDCONFIG "RELEASE"
#endif

#define MACRO_QUOTE(name) #name
#define MACRO_STR(macro) MACRO_QUOTE(macro)

namespace Engine
{
	// TODO: Move this abomination elsewhere...
	//
	const char* const _build = "CMEP EngineCore " __TIME__ " " __DATE__ " build, configured " BUILDCONFIG " " MACRO_STR(IS_DEBUG);
#if defined(_MSC_VER)
	#pragma message("Compiler MSVC detected")
	const char* const _compiledby = "MSVC " MACRO_STR(_MSC_FULL_VER) "." MACRO_STR(_MSC_BUILD);
#elif defined(__GNUC__)
	#pragma message("Compiler GNU-like detected")
	#if defined(__llvm__ )
		#pragma message("Compiler LLVM detected")
		#if defined(__clang__)
			#pragma message("Compiler LLVM-clang detected")
			const char* const _compiledby = "LLVM-clang " MACRO_STR(__clang_major__) "." MACRO_STR(__clang_minor__) "." MACRO_STR(__clang_patchlevel__);
		#else
			#pragma message("Compiler LLVM-gcc detected")
			const char* const _compiledby = "LLVM-GCC " MACRO_STR(__GNUC__) "." MACRO_STR(__GNUC_MINOR__) "." MACRO_STR(__GNUC_PATCHLEVEL__);
		#endif
	#else
		#pragma message("Compiler gcc detected")
		const char* const _compiledby = "GCC " MACRO_STR(__GNUC__) "." MACRO_STR(__GNUC_MINOR__) "." MACRO_STR(__GNUC_PATCHLEVEL__);
	#endif
#else
	#pragma warning "Compiler could not be identified"
	const char* const _compiledby = "Nil";
#endif

	// TODO: Does this have to be global?
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

		static double lastX = (windowdata.windowX / 2.0), lastY = (windowdata.windowY / 2.0);

		if (EngineIsWindowInFocus && EngineIsWindowInContent)
		{
			if ((EngineMouseXPos - lastX) != 0.0 || (EngineMouseYPos - lastY) != 0.0)
			{
				EventHandling::Event event = EventHandling::Event(EventHandling::EventType::ON_MOUSEMOVED);
				event.mouse.x = EngineMouseXPos - lastX;
				event.mouse.y = EngineMouseYPos - lastY;
				event.deltaTime = deltaTime;
				event.raisedFrom = this;
				this->FireEvent(event);

				lastX = EngineMouseXPos;
				lastY = EngineMouseYPos;
			}
		}
	}

	void Engine::HandleConfig()
	{
		std::ifstream file(this->config_path);
		
		this->config = std::make_unique<EngineConfig>();

		nlohmann::json data;
		try
		{
			data = nlohmann::json::parse(file);
		}
		catch(std::exception& e)
		{
			this->logger->SimpleLog(Logging::LogLevel::Exception, LOGPFX_CURRENT "Error parsing config json '%s', e.what(): %s", this->config_path.c_str(), e.what());
			exit(1);
		}

		this->config->window.title = data["window"]["title"];
		this->config->window.sizeX = data["window"]["sizeX"];
		this->config->window.sizeY = data["window"]["sizeY"];

		this->config->rendering.framerateTarget = data["rendering"]["framerateTarget"];
	
		this->config->lookup.scenes = data["lookup"]["scenes"];

		this->config->defaultScene = data["defaultScene"];
	}

	void Engine::ErrorCallback(int code, const char* message)
	{
		printf("GLFW ERROR: %u, %s\n", code, message);
	}
	
	void Engine::OnWindowFocusCallback(GLFWwindow* window, int focused)
	{
		// Unused
		(void)(window);

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
		// Unused
		(void)(window);

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
		// TODO: use entered?
		(void)(entered);

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
		// Unused
		(void)(scancode);
		(void)(mods);

		if (action == GLFW_PRESS)
		{
			Rendering::VulkanRenderingEngine* renderer = (Rendering::VulkanRenderingEngine*)glfwGetWindowUserPointer(window);
			EventHandling::Event event = EventHandling::Event(EventHandling::EventType::ON_KEYDOWN);
			event.keycode = static_cast<uint16_t>(key);
			event.deltaTime = renderer->GetOwnerEngine()->GetLastDeltaTime();
			event.raisedFrom = renderer->GetOwnerEngine();
			renderer->GetOwnerEngine()->FireEvent(event);
		}
		else if(action == GLFW_RELEASE)
		{
			Rendering::VulkanRenderingEngine* renderer = (Rendering::VulkanRenderingEngine*)glfwGetWindowUserPointer(window);
			EventHandling::Event event = EventHandling::Event(EventHandling::EventType::ON_KEYUP);
			event.keycode = static_cast<uint16_t>(key);
			event.deltaTime = renderer->GetOwnerEngine()->GetLastDeltaTime();
			event.raisedFrom = renderer->GetOwnerEngine();
			renderer->GetOwnerEngine()->FireEvent(event);
		}
	}

	void Engine::RenderCallback(VkCommandBuffer commandBuffer, uint32_t currentFrame, Engine* engine)
	{
		//engine->scene_manager->GetSceneCurrent()->TriggerResort();

		auto objects = engine->scene_manager->GetSceneCurrent()->GetAllObjectsSorted();

		for (auto& [name, ptr] : *objects)
		{
			try
			{
				ptr->renderer->Render(commandBuffer, currentFrame);
			}
			catch (const std::exception& e)
			{
				engine->logger->SimpleLog(Logging::LogLevel::Exception, LOGPFX_CURRENT "Caught exception while rendering object %s: %s", name.c_str(), e.what());
				exit(1);
			}
		}
	}

	void Engine::engineLoop()
	{	
		Object* object = new Object();
		object->renderer = new Rendering::AxisRenderer(this);
		object->Translate(glm::vec3(0, 0, 0));
		object->Scale(glm::vec3(1, 1, 1));
		object->Rotate(glm::vec3(0, 0, 0));
		object->ScreenSizeInform(this->config->window.sizeX, this->config->window.sizeY);
		((Rendering::AxisRenderer*)object->renderer)->UpdateMesh();
		((Rendering::AxisRenderer*)object->renderer)->scene_manager = this->scene_manager;
		this->scene_manager->AddObject("_axis", object);
		
		// Pre-make ON_UPDATE event so we don't have to create it over and over again in hot loop
		EventHandling::Event premadeOnUpdateEvent = EventHandling::Event(EventHandling::EventType::ON_UPDATE);
		premadeOnUpdateEvent.raisedFrom = this;
		
		glfwShowWindow(this->rendering_engine->GetWindow().window);

		uint16_t counter = 0;
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
			
			//glfwSwapBuffers(this->rendering_engine->GetWindow().window);
			glfwPollEvents();
			
			// spin sleep if framerate locked
			if(this->framerateTarget == 0)
			{
				this->framerateTarget = 5000;
			}

			const auto frameClock = std::chrono::steady_clock::now();
			const double sleepSecs = 1.0 / this->framerateTarget - (frameClock - nextClock).count() / 1e9;
			if (sleepSecs > 0 && framerateTarget != 0) // spin sleep if sleep necessary and VSYNC disabled
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

		// Allow binary handlers
		auto handler_range = this->event_handlers.equal_range(event.event_type); 
		for(auto handler = handler_range.first; handler != handler_range.second; ++handler)
		{
			sum += handler->second(event);
		}

		auto lua_handler_range = this->scene_manager->GetSceneCurrent()->lua_event_handlers.equal_range(event.event_type);
		for(auto handler = lua_handler_range.first; handler != lua_handler_range.second; ++handler)
		{
			sum += this->script_executor->CallIntoScript(Scripting::ExecuteType::EventHandler, handler->second.first, handler->second.second, &event);
		}
		return sum;
	}

	double Engine::GetLastDeltaTime()
	{
		return this->lastDeltaTime;
	}

	Engine::Engine(std::shared_ptr<Logging::Logger> logger) noexcept : logger(logger)
	{
	}

	Engine::~Engine() noexcept
	{
		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Destructor called");

		this->asset_manager.reset();

		delete this->script_executor;

		this->scene_manager.reset();

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
		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT
			"build info:\n////\nRunning %s\nCompiled by %s\n////\n", _build, _compiledby
		);

		// Load configuration
		try
		{
			this->HandleConfig();
		}
		catch(std::exception e)
		{
			this->logger->SimpleLog(Logging::LogLevel::Exception, LOGPFX_CURRENT "Exception parsing config! e.what(): %s", e.what());
			throw;
		}

		//return;
		// Order matters here due to interdependency

		this->script_executor = new Scripting::LuaScriptExecutor();
		// Will LuaScriptExecutor need to access owner engine? UpdateOwnerEngine
		this->script_executor->UpdateOwnerEngine(this);
		this->script_executor->UpdateHeldLogger(this->logger);

		this->asset_manager = std::make_shared<AssetManager>();
		//this->asset_manager->current_load_path = this->config.lookup.scenes + std::string("/") + this->config.defaultScene + std::string("/");
		this->asset_manager->UpdateOwnerEngine(this);
		this->asset_manager->UpdateHeldLogger(this->logger);

		this->asset_manager->lua_executor = this->script_executor;

		this->scene_manager = std::make_shared<SceneManager>(this->logger);
		this->scene_manager->UpdateOwnerEngine(this);
		this->scene_manager->UpdateHeldLogger(this->logger);
		this->scene_manager->SetSceneLoadPrefix(this->config->lookup.scenes);

		this->rendering_engine = new Rendering::VulkanRenderingEngine();
		this->rendering_engine->UpdateOwnerEngine(this);
		this->rendering_engine->UpdateHeldLogger(this->logger);
	}

	void Engine::Run()
	{
		auto start = std::chrono::steady_clock::now();

		// Initialize rendering engine
		this->rendering_engine->init(this->config->window.sizeX, this->config->window.sizeY, this->config->window.title);

		this->vulkanImageFactory = std::make_shared<Rendering::Factories::VulkanImageFactory>(this->rendering_engine->GetVMAAllocator(), this->rendering_engine);
		
		// Prepare rendering engine to run (framebuffers etc.)
		this->rendering_engine->prepRun();
		this->rendering_engine->SetRenderCallback(this->RenderCallback);

		//return;

		this->scene_manager->LoadScene(this->config->defaultScene);
		this->scene_manager->SetScene(this->config->defaultScene);

		// Set-up GLFW
		Rendering::GLFWwindowData windowdata = this->rendering_engine->GetWindow();
		glfwSetWindowFocusCallback(windowdata.window, Engine::OnWindowFocusCallback);
		glfwSetCursorPosCallback(windowdata.window, Engine::CursorPositionCallback);
		glfwSetCursorEnterCallback(windowdata.window, Engine::CursorEnterLeaveCallback);
		glfwSetKeyCallback(windowdata.window, Engine::OnKeyEventCallback);
		glfwSetErrorCallback(Engine::ErrorCallback);
		
		// Fire ON_INIT event
		EventHandling::Event onInitEvent = EventHandling::Event(EventHandling::EventType::ON_INIT);
		onInitEvent.raisedFrom = this;
		int onInitEventRet = this->FireEvent(onInitEvent);

		// Measure and log ON_INIT time
		double total = (std::chrono::steady_clock::now() - start).count() / 1e6;
		this->logger->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "Handling ON_INIT took %.3lf ms total and returned %i", total, onInitEventRet);

		//return;

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
		this->event_handlers.emplace(event_type, function);
		
	}
}