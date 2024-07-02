// #include <cassert>
#include "Engine.hpp"

#include "Assets/AssetManager.hpp"
#include "Rendering/AxisRenderer.hpp"
#include "Rendering/IRenderer.hpp"
#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"
#include "Rendering/Vulkan/VulkanStructDefs.hpp"

#include "Scripting/LuaScript.hpp"
#include "Scripting/LuaScriptExecutor.hpp"

#include "Logging/Logging.hpp"

#include "GLFW/glfw3.h"
// #include "IModule.hpp"
#include "Object.hpp"
#include "buildinfo.hpp"
#include "nlohmann/json.hpp"

#include <chrono>
#include <exception>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <thread>

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_ENGINE
#include "Logging/LoggingPrefix.hpp"

namespace Engine
{
	// Utility sleep function
	void Engine::SpinSleep(double seconds)
	{
		static constexpr double nano_to_sec = 1e9;
		static constexpr double spin_init	= 5e-3;

		static double estimate = spin_init;
		static double mean	   = spin_init;
		static double m2	   = 0;
		static int64_t count   = 1;

		while (seconds > estimate)
		{
			const auto start = std::chrono::steady_clock::now();
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
			const auto end = std::chrono::steady_clock::now();

			const double observed = static_cast<double>((end - start).count()) / nano_to_sec;
			seconds -= observed;

			count++;
			const double delta = observed - mean;
			mean += delta / static_cast<double>(count);
			m2 += delta * (observed - mean);
			const double stddev = sqrt(m2 / static_cast<double>(count - 1));
			estimate			= mean + stddev;
		}

		// spin lock
		const auto start = std::chrono::steady_clock::now();
		while (static_cast<double>((std::chrono::steady_clock::now() - start).count()) / nano_to_sec < seconds)
		{
		}
	}

	void Engine::HandleInput(const double deltaTime) noexcept
	{
		Rendering::GLFWwindowData windowdata = this->rendering_engine->GetWindow();

		static double last_x = (windowdata.window_x / 2);
		static double last_y = (windowdata.window_y / 2);

		if (this->state_is_window_in_focus && this->state_is_window_in_content)
		{
			if ((this->state_mouse_x_pos - last_x) != 0.0 || (this->state_mouse_y_pos - last_y) != 0.0)
			{
				auto event		  = EventHandling::Event(EventHandling::EventType::ON_MOUSEMOVED);
				event.mouse.x	  = this->state_mouse_x_pos - last_x;
				event.mouse.y	  = this->state_mouse_y_pos - last_y;
				event.delta_time  = deltaTime;
				event.raised_from = this;
				this->FireEvent(event);

				last_x = this->state_mouse_x_pos;
				last_y = this->state_mouse_y_pos;
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
		catch (std::exception& e)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Exception,
				LOGPFX_CURRENT "Error parsing config json '%s', e.what(): %s",
				this->config_path.c_str(),
				e.what()
			);
			exit(1);
		}

		this->config->window.title	= data["window"]["title"].get<std::string>();
		this->config->window.size_x = data["window"]["sizeX"].get<uint16_t>();
		this->config->window.size_y = data["window"]["sizeY"].get<uint16_t>();

		this->config->rendering.framerate_target = data["rendering"]["framerateTarget"].get<uint16_t>();

		this->config->lookup.scenes = data["lookup"]["scenes"].get<std::string>();

		this->config->default_scene = data["defaultScene"].get<std::string>();
	}

	void Engine::ErrorCallback(int code, const char* message)
	{
		printf("GLFW ERROR: %u, %s\n", code, message);
	}

	void Engine::OnWindowFocusCallback(GLFWwindow* window, int focused)
	{
		auto* engine = static_cast<Rendering::VulkanRenderingEngine*>(glfwGetWindowUserPointer(window))
						   ->GetOwnerEngine();

		engine->state_is_window_in_focus = (focused != 0);
	}

	void Engine::CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
	{
		auto* engine = static_cast<Rendering::VulkanRenderingEngine*>(glfwGetWindowUserPointer(window))
						   ->GetOwnerEngine();

		if (engine->state_is_window_in_focus)
		{
			engine->state_mouse_x_pos = xpos;
			engine->state_mouse_y_pos = ypos;
		}
		else
		{
			engine->state_mouse_x_pos = 0.0;
			engine->state_mouse_y_pos = 0.0;
		}
	}

	void Engine::CursorEnterLeaveCallback(GLFWwindow* window, int entered)
	{
		bool b_entered = (entered != 0);

		auto* engine = static_cast<Rendering::VulkanRenderingEngine*>(glfwGetWindowUserPointer(window))
						   ->GetOwnerEngine();

		if (engine->state_is_window_in_focus)
		{
			engine->state_is_window_in_content = b_entered;
			glfwSetInputMode(window, GLFW_CURSOR, b_entered ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
		}
		else
		{
			engine->state_is_window_in_content = false;
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
			auto* renderer	  = static_cast<Rendering::VulkanRenderingEngine*>(glfwGetWindowUserPointer(window));
			auto event		  = EventHandling::Event(EventHandling::EventType::ON_KEYDOWN);
			event.keycode	  = static_cast<uint16_t>(key);
			event.delta_time  = renderer->GetOwnerEngine()->GetLastDeltaTime();
			event.raised_from = renderer->GetOwnerEngine();
			renderer->GetOwnerEngine()->FireEvent(event);
		}
		else if (action == GLFW_RELEASE)
		{
			auto* renderer	  = static_cast<Rendering::VulkanRenderingEngine*>(glfwGetWindowUserPointer(window));
			auto event		  = EventHandling::Event(EventHandling::EventType::ON_KEYUP);
			event.keycode	  = static_cast<uint16_t>(key);
			event.delta_time  = renderer->GetOwnerEngine()->GetLastDeltaTime();
			event.raised_from = renderer->GetOwnerEngine();
			renderer->GetOwnerEngine()->FireEvent(event);
		}
	}

	void Engine::RenderCallback(VkCommandBuffer commandBuffer, uint32_t currentFrame, Engine* engine)
	{
		const auto* objects = engine->scene_manager->GetSceneCurrent()->GetAllObjectsSorted();

		for (const auto& [name, ptr] : *objects)
		{
			try
			{
				ptr->GetRenderer()->Render(commandBuffer, currentFrame);
			}
			catch (const std::exception& e)
			{
				engine->logger->SimpleLog(
					Logging::LogLevel::Exception,
					LOGPFX_CURRENT "Caught exception while rendering object '%s'! e.what(): %s",
					name.c_str(),
					e.what()
				);
				throw;
			}
		}
	}

	void Engine::EngineLoop()
	{
		// TODO: Remove this!
		// Create axis object
		auto* object = new Object(this);

		Rendering::IRenderer* with_renderer = new Rendering::AxisRenderer(this);

		object->SetRenderer(with_renderer);

		object->SetPosition(glm::vec3(0, 0, 0));
		object->SetSize(glm::vec3(1, 1, 1));
		object->SetRotation(glm::vec3(0, 0, 0));
		object->ScreenSizeInform(this->config->window.size_x, this->config->window.size_y);

		this->scene_manager->AddObject("_axis", object);

		// Pre-make ON_UPDATE event so we don't have to create it over and over again in hot loop
		auto premade_on_update_event		= EventHandling::Event(EventHandling::EventType::ON_UPDATE);
		premade_on_update_event.raised_from = this;

		this->logger->SimpleLog(
			Logging::LogLevel::Debug1,
			LOGPFX_CURRENT "Locked to framerate %u%s",
			this->framerate_target,
			this->framerate_target == 0 ? " (VSYNC)" : ""
		);

		// Show window
		Rendering::GLFWwindowData glfw_window = this->rendering_engine->GetWindow();
		glfwShowWindow(glfw_window.window);

		// static constexpr double nano_to_msec = 1.e6;
		static constexpr double nano_to_sec = 1e9;

		auto prev_clock = std::chrono::steady_clock::now();

		// hot loop
		while (glfwWindowShouldClose(glfw_window.window) == 0)
		{
			const auto next_clock	= std::chrono::steady_clock::now();
			const double delta_time = static_cast<double>((next_clock - prev_clock).count()) / nano_to_sec;
			this->last_delta_time	= delta_time;

			// Update deltaTime of premade ON_UPDATE event and fire it
			premade_on_update_event.delta_time = delta_time;

			// Check return code of FireEvent (events should return non-zero codes as failure)
			const auto ret = this->FireEvent(premade_on_update_event);
			if (ret != 0)
			{
				this->logger->SimpleLog(
					Logging::LogLevel::Error,
					LOGPFX_CURRENT "Fired event ON_UPDATE but it returned %u! Exiting event-loop",
					ret
				);
				break;
			}

			// this->logger->SimpleLog(Logging::LogLevel::Info, "FT %lf", 0.016666 / delta_time);

			// Render
			this->rendering_engine->DrawFrame();

			// Sync with glfw event loop
			glfwPollEvents();

			const auto frame_clock	= std::chrono::steady_clock::now();
			const double sleep_secs = 1.0 / this->framerate_target -
									  static_cast<double>((frame_clock - next_clock).count()) / nano_to_sec;
			// spin sleep if sleep necessary and VSYNC disabled
			if (sleep_secs > 0 && this->framerate_target != 0)
			{
				SpinSleep(sleep_secs);
			}

			prev_clock = next_clock;
		}

		this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Closing engine");
	}

	int Engine::FireEvent(EventHandling::Event& event)
	{
		int sum = 0;

		// Allow binary handlers
		// auto handler_range = this->event_handlers.equal_range(event.event_type);
		// for (auto handler = handler_range.first; handler != handler_range.second; ++handler)
		//{
		//	sum += handler->second(event);
		//}

		auto current_scene	   = this->scene_manager->GetSceneCurrent();
		auto lua_handler_range = current_scene->lua_event_handlers.equal_range(event.event_type);
		for (auto handler = lua_handler_range.first; handler != lua_handler_range.second; ++handler)
		{
			try
			{
				sum += this->script_executor->CallIntoScript(
					Scripting::ExecuteType::EVENT_HANDLER,
					handler->second.first,
					handler->second.second,
					&event
				);
			}
			catch (std::runtime_error& e)
			{
				this->logger->SimpleLog(
					Logging::LogLevel::Exception,
					"Caught exception trying to fire event '%u'! e.what(): %s",
					event.event_type,
					e.what()
				);
			}
		}

		return sum;
	}

	double Engine::GetLastDeltaTime() const
	{
		return this->last_delta_time;
	}

	Engine::Engine(std::shared_ptr<Logging::Logger>& with_logger) noexcept : logger(with_logger)
	{
	}

	Engine::~Engine() noexcept
	{
		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Destructor called");

		this->scene_manager.reset();

		delete this->script_executor;

		this->asset_manager.reset();

		this->rendering_engine->Cleanup();

		delete this->rendering_engine;
	}

	void Engine::Init()
	{
		this->logger->MapCurrentThreadToName("engine");

		// Engine info printout
		this->logger->SimpleLog(
			Logging::LogLevel::Info,
			LOGPFX_CURRENT "build info:\n////\nRunning %s\nCompiled by %s\n////\n",
			buildinfo_build,
			buildinfo_compiledby
		);

		// Load configuration
		try
		{
			this->HandleConfig();
		}
		catch (std::exception& e)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Exception,
				LOGPFX_CURRENT "Exception parsing config! e.what(): %s",
				e.what()
			);
			throw;
		}

		// return;
		// Order matters here due to interdependency

		this->script_executor = new Scripting::LuaScriptExecutor(this);

		this->asset_manager = std::make_shared<AssetManager>(this);

		this->asset_manager->lua_executor = this->script_executor;

		this->scene_manager = std::make_shared<SceneManager>(this);
		this->scene_manager->SetSceneLoadPrefix(this->config->lookup.scenes);

		this->rendering_engine = new Rendering::VulkanRenderingEngine(this);
	}

	void Engine::Run()
	{
		auto start = std::chrono::steady_clock::now();

		// Initialize rendering engine
		this->rendering_engine
			->Init(this->config->window.size_x, this->config->window.size_y, this->config->window.title);

		this->vulkan_image_factory = std::make_shared<Rendering::Factories::VulkanImageFactory>(
			this,
			this->rendering_engine->GetVMAAllocator(),
			this->rendering_engine
		);

		// Prepare rendering engine to run (framebuffers etc.)
		this->rendering_engine->PrepRun();
		this->rendering_engine->SetRenderCallback(Engine::Engine::RenderCallback);

		// return;

		this->scene_manager->LoadScene(this->config->default_scene);
		this->scene_manager->SetScene(this->config->default_scene);

		// Set-up GLFW
		Rendering::GLFWwindowData windowdata = this->rendering_engine->GetWindow();
		glfwSetWindowFocusCallback(windowdata.window, Engine::OnWindowFocusCallback);
		glfwSetCursorPosCallback(windowdata.window, Engine::CursorPositionCallback);
		glfwSetCursorEnterCallback(windowdata.window, Engine::CursorEnterLeaveCallback);
		glfwSetKeyCallback(windowdata.window, Engine::OnKeyEventCallback);
		glfwSetErrorCallback(Engine::ErrorCallback);

		// Fire ON_INIT event
		auto on_init_event		  = EventHandling::Event(EventHandling::EventType::ON_INIT);
		on_init_event.raised_from = this;
		int on_init_event_ret	  = this->FireEvent(on_init_event);

		// Measure and log ON_INIT time
		static constexpr double nano_to_msec = 1.e6;
		double total = static_cast<double>((std::chrono::steady_clock::now() - start).count()) / nano_to_msec;
		this->logger->SimpleLog(
			Logging::LogLevel::Debug1,
			LOGPFX_CURRENT "Handling ON_INIT took %.3lf ms total and returned %i",
			total,
			on_init_event_ret
		);

		// return;

		if (on_init_event_ret != 0)
		{
			return;
		}

		this->EngineLoop();
	}

	[[noreturn]] void Engine::ThrowTest()
	{
		throw std::runtime_error("BEBEACAC");
	}

	void Engine::Stop()
	{
		// 1 denotes true here
		glfwSetWindowShouldClose(this->rendering_engine->GetWindow().window, 1);
	}

	void Engine::ConfigFile(std::string path)
	{
		this->config_path = std::move(path);
	}
	/*
		void Engine::RegisterEventHandler(
			EventHandling::EventType event_type, std::function<int(EventHandling::Event&)> function
		)
		{
			this->event_handlers.emplace(event_type, function);
		}
	*/
} // namespace Engine
