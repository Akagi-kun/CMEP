#include <assert.h>
#include <fstream>
#include <memory>
#include <chrono>

#include "nlohmann-json/json.hpp"

#include "Scripting/LuaScriptExecutor.hpp"
#include "Scripting/LuaScript.hpp"

#include "AssetManager.hpp"
#include "Engine.hpp"
#include "Object.hpp"

#if defined( _WIN32 )
	#ifndef _DEBUG
		#define _DEBUG 0
	#endif
#endif

namespace Engine
{
	void APIENTRY Engine::debugCallbackGL(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) noexcept
	{
		const char* _source;
		const char* _type;
		const char* _severity;

		switch (source) {
		case GL_DEBUG_SOURCE_API:
			_source = "API";
			break;

		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
			_source = "WINDOW SYSTEM";
			break;

		case GL_DEBUG_SOURCE_SHADER_COMPILER:
			_source = "SHADER COMPILER";
			break;

		case GL_DEBUG_SOURCE_THIRD_PARTY:
			_source = "THIRD PARTY";
			break;

		case GL_DEBUG_SOURCE_APPLICATION:
			_source = "APPLICATION";
			break;

		case GL_DEBUG_SOURCE_OTHER:
			_source = "UNKNOWN";
			break;

		default:
			_source = "UNKNOWN";
			break;
		}

		switch (type) {
		case GL_DEBUG_TYPE_ERROR:
			_type = "ERROR";
			break;

		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			_type = "DEPRECATED BEHAVIOR";
			break;

		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			_type = "UDEFINED BEHAVIOR";
			break;

		case GL_DEBUG_TYPE_PORTABILITY:
			_type = "PORTABILITY";
			break;

		case GL_DEBUG_TYPE_PERFORMANCE:
			_type = "PERFORMANCE";
			break;

		case GL_DEBUG_TYPE_OTHER:
			_type = "OTHER";
			break;

		case GL_DEBUG_TYPE_MARKER:
			_type = "MARKER";
			break;

		default:
			_type = "UNKNOWN";
			break;
		}

		switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH:
			_severity = "HIGH";
			break;

		case GL_DEBUG_SEVERITY_MEDIUM:
			_severity = "MEDIUM";
			break;

		case GL_DEBUG_SEVERITY_LOW:
			_severity = "LOW";
			break;

		case GL_DEBUG_SEVERITY_NOTIFICATION:
			_severity = "NOTIFICATION";
			break;

		default:
			_severity = "UNKNOWN";
			break;
		}

		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug1, "OpenGL debug message received: %d %s of %s severity, raised from %s: %s\n\n", id, _type, _severity, _source, message);
	}

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
		double xpos, ypos;

		glfwGetCursorPos(window, &xpos, &ypos);

		if (glfwGetWindowAttrib(window, GLFW_FOCUSED))
		{
			if ((this->windowX / 2) - xpos != 0 || (this->windowY / 2) - ypos != 0)
			{
				EventHandling::Event event = EventHandling::Event(EventHandling::EventType::ON_MOUSEMOVED);
				event.mouse.x = xpos;
				event.mouse.y = ypos;
				event.deltaTime = deltaTime;
				this->FireEvent(event);

				glfwSetCursorPos(this->window, this->windowX / 2, this->windowY / 2);
			}
		}

		// Handle keyboard + mouse buttons
		for (uint16_t i = 1; i < 256; i++)
		{
			if (glfwGetKey(this->window, i))
			{
				EventHandling::Event event = EventHandling::Event(EventHandling::EventType::ON_KEYDOWN);
				event.keycode = i;
				event.deltaTime = deltaTime;
				this->FireEvent(event);
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
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Exception, "Error parsing config json, what: %s", e.what());
			exit(1);
		}

		Scripting::LuaScript* event_handler;
		
		std::string setting = "eventHandler.onInit";
		asset_manager->AddLuaScript(data[setting]["file"], data[setting]["file"]);
		event_handler = asset_manager->GetLuaScript(data[setting]["file"]);
		global_engine->RegisterLuaEventHandler(::Engine::EventHandling::EventType::ON_INIT, event_handler, data[setting]["function"]);

		setting = "eventHandler.onUpdate";
		asset_manager->AddLuaScript(data[setting]["file"], data[setting]["file"]);
		event_handler = asset_manager->GetLuaScript(data[setting]["file"]);
		global_engine->RegisterLuaEventHandler(::Engine::EventHandling::EventType::ON_UPDATE, event_handler, data[setting]["function"]);

		setting = "eventHandler.onKeyDown";
		asset_manager->AddLuaScript(data[setting]["file"], data[setting]["file"]);
		event_handler = asset_manager->GetLuaScript(data[setting]["file"]);
		global_engine->RegisterLuaEventHandler(::Engine::EventHandling::EventType::ON_KEYDOWN, event_handler, data[setting]["function"]);

		setting = "eventHandler.onMouseMoved";
		asset_manager->AddLuaScript(data[setting]["file"], data[setting]["file"]);
		event_handler = asset_manager->GetLuaScript(data[setting]["file"]);
		global_engine->RegisterLuaEventHandler(::Engine::EventHandling::EventType::ON_MOUSEMOVED, event_handler, data[setting]["function"]);

		setting = "window.title";
		this->windowTitle = data[setting];
		glfwSetWindowTitle(this->window, this->windowTitle.c_str());

		setting = "window.size";
		this->windowX = data[setting]["x"]; this->windowY = data[setting]["y"];
		glfwSetWindowSize(this->window, this->windowX, this->windowY);
	}

	void Engine::engineLoop()
	{
		Logging::GlobalLogger->MapCurrentThreadToName("game");

		glEnable(GL_MULTISAMPLE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glViewport(0, 0, this->windowX, this->windowY);

		// Pre-make ON_UPDATE event so we don't have to create it over and over again in hot loop
		EventHandling::Event premadeOnUpdateEvent = EventHandling::Event(EventHandling::EventType::ON_UPDATE);
		
		glfwShowWindow(this->window);

		int counter = 0;
		auto prevClock = std::chrono::steady_clock::now();
		// hot loop
		while (!glfwWindowShouldClose(this->window))
		{
			const auto nextClock = std::chrono::steady_clock::now();
			const double deltaTime = (nextClock - prevClock).count() / 1e9;
			if (counter == this->framerateTarget)
			{
				// For debugging, use onscreen counter if possible
				//printf("current frame time: %.2lf ms (%.1lf fps)\n", deltaTime * 1e3, (1 / (deltaTime)));
				counter = 0;
			}

			// Handle input
			this->handleInput(deltaTime);

			// Update deltaTime of premade ON_UPDATE event and fire it
			premadeOnUpdateEvent.deltaTime = deltaTime;
			this->FireEvent(premadeOnUpdateEvent);

			// Clear screen
			glClearColor(0.1, 0.13, 0.2, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Render every object
			for (auto& [name, ptr] : *global_scene_manager->GetAllObjects())
			{
				try
				{
					ptr->renderer->Render();
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				}
				catch (const std::exception& e)
				{
					Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Exception, "Caught exception while rendering object %s: %s", name.c_str(), e.what());
					exit(1);
				}
			}
			
			enum { EASY, HARD };
			static int op = EASY;
			static float value = 0.6f;
			static int i = 20;

			glfwSwapBuffers(this->window);
			glfwPollEvents();
			
			// spin sleep if framerate locked
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

	void Engine::FireEvent(EventHandling::Event& event)
	{
		for (auto handler : this->event_handlers)
		{
			if (handler.first == event.event_type)
			{
				handler.second(event);
			}
		}

		for (auto handler : this->lua_event_handlers)
		{
			if (std::get<0>(handler) == event.event_type)
			{
				this->script_executor->CallIntoScript(Scripting::ExecuteType::EventHandler, std::get<1>(handler), std::get<2>(handler), &event);
			}
		}
	}

	Engine::Engine(const char* windowTitle, const unsigned windowX, const unsigned windowY) noexcept : windowTitle(windowTitle), windowX(windowX), windowY(windowY), framerateTarget(30) {}

	Engine::~Engine() noexcept
	{
		glfwTerminate();
		
		//delete this->window;
		delete this->asset_manager;
	}

	void Engine::SetFramerateTarget(unsigned framerate) noexcept
	{
		this->framerateTarget = framerate;
	}

	void Engine::Init()
	{
		Logging::GlobalLogger->MapCurrentThreadToName("main");

		// Engine info printout
		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Info, 
			"Engine info:\n////\nRunning CMEP EngineCore %s %s build, configured %s\nCompiler version: %u.%u\n////\n", 
			__TIME__, __DATE__, _DEBUG ? "DEBUG" : "RELEASE", _MSC_FULL_VER, _MSC_BUILD
		);

		//this->window = new Rendering::Window(this->windowTitle, this->windowX, this->windowY);
		this->asset_manager = new AssetManager();
		this->script_executor = new Scripting::LuaScriptExecutor();
		
		if (!glfwInit())
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "glfwInit returned 0!");
			exit(1);
		}
		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Info, "GLFW initialized");

		glfwWindowHint(GLFW_SAMPLES, 4);
		glfwWindowHint(GLFW_DEPTH_BITS, 16);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		this->window = glfwCreateWindow(this->windowX, this->windowY, this->windowTitle.c_str(), NULL, NULL);

		glfwMakeContextCurrent(this->window);
		glfwSwapInterval(1);

		if (glewInit() != GLEW_OK)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "glewInit returned 0!");
			exit(1);
		}
		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Info, "GLEW initialized");

		GLint maxTextures;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextures);
		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug1, "Texture limit: %i", maxTextures);

		// OpenGL info printout
		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Info, "OpenGL info:\n---------- OpenGL ----------\n Vendor: %s\n Renderer: %s\n Version: %s\n----------------------------\n",
			(char*)glGetString(GL_VENDOR), (char*)glGetString(GL_RENDERER), (char*)glGetString(GL_VERSION)
		);

		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Info, "Setting OpenGL debug callback");
		glDebugMessageCallback(Engine::debugCallbackGL, nullptr);
	}

	void Engine::Run()
	{
		auto start = std::chrono::steady_clock::now();
		
		//this->script_executor->ConfigScriptLoad(this->config_script);

		this->HandleConfig();

		// Fire ON_INIT event
		EventHandling::Event onInitEvent = EventHandling::Event(EventHandling::EventType::ON_INIT);
		this->FireEvent(onInitEvent);

		// Measure and log ON_INIT time
		double total = (std::chrono::steady_clock::now() - start).count() / 1e6;
		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug1, "Handling ON_INIT took %.3lf ms total", total);

		this->engineLoop();
	}

	void Engine::ConfigFile(std::string path)
	{
		this->config_path = path;
	}

	void Engine::RegisterEventHandler(EventHandling::EventType event_type, std::function<void(EventHandling::Event&)> function)
	{
		this->event_handlers.push_back(std::make_pair(event_type, function));
	}

	void Engine::RegisterLuaEventHandler(EventHandling::EventType event_type, Scripting::LuaScript* script, std::string function)
	{
		this->lua_event_handlers.push_back(std::make_tuple(event_type, script, function));
	}

	void Engine::AddObject(std::string name, Object* ptr)
	{
		if (ptr != nullptr)
		{
			ptr->ScreenSizeInform(this->windowX, this->windowY);
			global_scene_manager->AddObject(name, ptr);
		}
	}

	Object* Engine::FindObject(std::string name)
	{
		return global_scene_manager->FindObject(name);
	}

	size_t Engine::RemoveObject(std::string name) noexcept
	{
		return global_scene_manager->RemoveObject(name);
	}

	AssetManager* Engine::GetAssetManager() noexcept
	{
		return this->asset_manager;
	}

	Engine* initializeEngine(const char* windowTitle, const unsigned windowX, const unsigned windowY)
	{
		// Set up loggre
		Logging::GlobalLogger = std::make_unique<Logging::Logger>();
#if _DEBUG == 1
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

	__declspec(dllexport) Engine* global_engine;
}