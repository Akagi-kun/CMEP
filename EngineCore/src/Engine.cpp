#include "Engine.hpp"

#include "Assets/AssetManager.hpp"
#include "Rendering/AxisMeshBuilder.hpp"
#include "Rendering/IRenderer.hpp"
#include "Rendering/Renderer3D.hpp"
#include "Rendering/Vulkan/PipelineManager.hpp"
#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"
#include "Rendering/Vulkan/VulkanStructDefs.hpp"

#include "Scripting/ILuaScript.hpp"

#include "Factories/ObjectFactory.hpp"

#include "Logging/Logging.hpp"

#include "GLFW/glfw3.h"
#include "Object.hpp"
#include "buildinfo.hpp"
#include "nlohmann/json.hpp"
#include "vulkan/vulkan_core.h"

#include <chrono>
#include <cstdint>
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
	// algorithm I found somewhere on stackoverflow
	void Engine::SpinSleep(double seconds)
	{
		static constexpr double nano_to_sec = 1e9;
		static constexpr double spin_init	= 5e-3;

		double estimate	  = spin_init;
		double mean		  = spin_init;
		double sumsquares = 0; // also known as m2
		int64_t count	  = 1;

		while (seconds > estimate)
		{
			// Perform measured sleep
			const auto start = std::chrono::steady_clock::now();
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
			const auto end = std::chrono::steady_clock::now();

			// Observed passage of time (as was performed by sleep_for)
			const double observed = static_cast<double>((end - start).count()) / nano_to_sec;
			// Substract observed passage of time
			// from total needed amount of sleep
			seconds -= observed;

			count++;
			// change against mean average
			const double delta = observed - mean;
			// create new mean by adding current change to it
			mean += delta / static_cast<double>(count);
			// sum of squares accumulate
			sumsquares += delta * (observed - mean);
			// calculate standard deviation
			const double stddev = sqrt(sumsquares / static_cast<double>(count - 1));
			estimate			= mean + stddev;
		}

		// spin lock
		const auto start = std::chrono::steady_clock::now();
		while (static_cast<double>((std::chrono::steady_clock::now() - start).count()) / nano_to_sec < seconds)
		{
		}
	}

	void Engine::HandleInput(const double delta_time) noexcept
	{
		Rendering::GLFWwindowData windowdata = this->rendering_engine->GetWindow();

		static double last_x = static_cast<double>(windowdata.size.x) / 2;
		static double last_y = static_cast<double>(windowdata.size.y) / 2;

		static constexpr double clamp_difference = 128;

		if (this->state_is_window_in_focus && this->state_is_window_in_content)
		{
			if ((this->state_mouse_x_pos - last_x) != 0.0 || (this->state_mouse_y_pos - last_y) != 0.0)
			{
				auto event		 = EventHandling::Event(this, EventHandling::EventType::ON_MOUSEMOVED);
				event.mouse.x	 = std::clamp(this->state_mouse_x_pos - last_x, -clamp_difference, clamp_difference);
				event.mouse.y	 = std::clamp(this->state_mouse_y_pos - last_y, -clamp_difference, clamp_difference);
				event.delta_time = delta_time;
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
		this->config->window.size.x = data["window"]["sizeX"].get<uint16_t>();
		this->config->window.size.y = data["window"]["sizeY"].get<uint16_t>();

		this->config->framerate_target = data["rendering"]["framerateTarget"].get<uint16_t>();

		this->config->scene_path	= data["scene_path"].get<std::string>();
		this->config->default_scene = data["default_scene"].get<std::string>();

		this->config->shader_path = data["shader_path"].get<std::string>();
	}

	void Engine::ErrorCallback(int code, const char* message)
	{
		printf("GLFW ERROR: %u, %s\n", code, message);
	}

	void Engine::OnWindowFocusCallback(GLFWwindow* window, int focused)
	{
		auto* engine =
			static_cast<Rendering::Vulkan::VulkanRenderingEngine*>(glfwGetWindowUserPointer(window))->GetOwnerEngine();

		engine->state_is_window_in_focus = (focused != 0);
	}

	void Engine::CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
	{
		auto* engine =
			static_cast<Rendering::Vulkan::VulkanRenderingEngine*>(glfwGetWindowUserPointer(window))->GetOwnerEngine();

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

		auto* engine =
			static_cast<Rendering::Vulkan::VulkanRenderingEngine*>(glfwGetWindowUserPointer(window))->GetOwnerEngine();

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

		auto* renderer	   = static_cast<Rendering::Vulkan::VulkanRenderingEngine*>(glfwGetWindowUserPointer(window));
		auto* owner_engine = renderer->GetOwnerEngine();

		if (action == GLFW_PRESS || action == GLFW_REPEAT)
		{
			auto event		 = EventHandling::Event(owner_engine, EventHandling::EventType::ON_KEYDOWN);
			event.keycode	 = static_cast<uint16_t>(key);
			event.delta_time = owner_engine->GetLastDeltaTime();
			owner_engine->FireEvent(event);
		}
		else if (action == GLFW_RELEASE)
		{
			auto event		 = EventHandling::Event(owner_engine, EventHandling::EventType::ON_KEYUP);
			event.keycode	 = static_cast<uint16_t>(key);
			event.delta_time = owner_engine->GetLastDeltaTime();
			owner_engine->FireEvent(event);
		}
	}

	void Engine::RenderCallback(
		Rendering::Vulkan::VCommandBuffer* command_buffer,
		uint32_t current_frame,
		Engine* engine
	)
	{
		auto& current_scene = engine->scene_manager->GetSceneCurrent();

		//// If was_scene_modified is true here, the scene had been sorted
		const auto& objects = current_scene->GetAllObjects();

		// engine->logger->SimpleLog(Logging::LogLevel::Info, "Object count: %lu", objects->size());

		for (const auto& [name, ptr] : objects)
		{
			try
			{
				ptr->GetRenderer()->Render(command_buffer, current_frame);
			}
			catch (const std::exception& e)
			{
				engine->logger->SimpleLog(
					Logging::LogLevel::Exception,
					LOGPFX_CURRENT "Caught exception while rendering object! e.what(): %s",
					e.what()
				);
				throw;
			}
		}
	}

	void Engine::EngineLoop()
	{
		static constexpr double nano_to_msec = 1e6;
		static constexpr double nano_to_sec	 = 1e9;
		static constexpr double sec_to_msec	 = 1e3;

		auto& scene = this->scene_manager->GetSceneCurrent();

		// TODO: Remove this!
		// Create axis object
		auto* object = Factories::ObjectFactory::CreateSceneObject<Rendering::Renderer3D, Rendering::AxisMeshBuilder>(
			this,
			"axis",
			{}
		);

		scene->AddObject("_axis", object);

		// Pre-make ON_UPDATE event so we don't have to create it over and over again in hot loop
		auto on_update_event = EventHandling::Event(this, EventHandling::EventType::ON_UPDATE);

		this->logger->SimpleLog(
			Logging::LogLevel::Debug1,
			LOGPFX_CURRENT "Locked to framerate %u%s",
			this->config->framerate_target,
			this->config->framerate_target == 0 ? " (VSYNC)" : ""
		);

		auto build_clock = std::chrono::steady_clock::now();

		this->logger->SimpleLog(Logging::LogLevel::Info, "Starting scene build");
		for (const auto& [name, object] : scene->GetAllObjects())
		{
			this->logger->SimpleLog(Logging::LogLevel::Debug1, "Building object '%s'", name.c_str());
			object->GetRenderer()->ForceBuild();
		}

		auto scene_build_time = static_cast<double>((std::chrono::steady_clock::now() - build_clock).count()) /
								nano_to_msec;

		this->logger->SimpleLog(
			Logging::LogLevel::Info,
			"Scene build finished in %lums",
			static_cast<uint_fast32_t>(scene_build_time)
		);

		// Show window
		Rendering::GLFWwindowData glfw_window = this->rendering_engine->GetWindow();
		glfwShowWindow(glfw_window.native_handle);

		double average_event_total	 = 0.00;
		uint64_t average_event_count = 1;

		auto prev_clock = std::chrono::steady_clock::now();

		// hot loop
		while (glfwWindowShouldClose(glfw_window.native_handle) == 0)
		{
			const auto next_clock	= std::chrono::steady_clock::now();
			const double delta_time = static_cast<double>((next_clock - prev_clock).count()) / nano_to_sec;
			this->last_delta_time	= delta_time;

			this->HandleInput(delta_time);

			// Check return code of FireEvent (events should return non-zero codes as failure)
			on_update_event.delta_time = delta_time;
			const auto ret			   = this->FireEvent(on_update_event);
			if (ret != 0)
			{
				this->logger->SimpleLog(
					Logging::LogLevel::Error,
					LOGPFX_CURRENT "Firing event ON_UPDATE returned %u! Exiting event-loop",
					ret
				);
				break;
			}

			const auto event_clock = std::chrono::steady_clock::now();

			// Render
			this->rendering_engine->DrawFrame();

			const auto draw_clock = std::chrono::steady_clock::now();

			// Sync with glfw event loop
			glfwPollEvents();

			const auto poll_clock = std::chrono::steady_clock::now();

			const double event_time = static_cast<double>((event_clock - next_clock).count()) / nano_to_sec *
									  sec_to_msec;
			const double draw_time = static_cast<double>((draw_clock - event_clock).count()) / nano_to_sec *
									 sec_to_msec;
			const double poll_time = static_cast<double>((poll_clock - draw_clock).count()) / nano_to_sec * sec_to_msec;

			const auto time_sum = event_time + draw_time + poll_time;

			average_event_total += event_time;
			average_event_count++;

			constexpr double limits[] = {12.0, 19.0, 8.0};

			if (event_time > limits[0] || time_sum > limits[1] || time_sum < limits[2])
			{
				this->logger->SimpleLog(
					Logging::LogLevel::Warning,
					"delta %lf sum %lf (event %lf draw %lf poll %lf)",
					delta_time * sec_to_msec,
					time_sum,
					event_time,
					draw_time,
					poll_time
				);
			}

			/* const auto frame_clock	= std::chrono::steady_clock::now();
			const double sleep_secs = 1.0 / this->framerate_target -
									  static_cast<double>((frame_clock - next_clock).count()) / nano_to_sec;
			// spin sleep if sleep necessary and VSYNC disabled
			if (sleep_secs > 0 && this->framerate_target != 0)
			{
				SpinSleep(sleep_secs);
			} */

			prev_clock = next_clock;
		}

		this->logger->SimpleLog(
			Logging::LogLevel::Debug2,
			LOGPFX_CURRENT "Closing engine (eventtime avg %lf)",
			average_event_total / static_cast<double>(average_event_count)
		);
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
				sum += handler->second.first->CallFunction(handler->second.second, &event);
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

	Engine::Engine(std::shared_ptr<Logging::Logger>& with_logger) : logger(with_logger)
	{
	}

	Engine::~Engine()
	{
		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Destructor called");

		this->scene_manager.reset();

		this->asset_manager.reset();

		this->pipeline_manager.reset();

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

		// Order matters here due to interdependency

		this->asset_manager = std::make_shared<AssetManager>(this);

		this->scene_manager = std::make_shared<SceneManager>(this);
	}

	void Engine::Run()
	{
		auto start = std::chrono::steady_clock::now();

		// Initialize rendering engine
		this->rendering_engine =
			new Rendering::Vulkan::VulkanRenderingEngine(this, this->config->window.size, this->config->window.title);
		this->rendering_engine->SetRenderCallback(Engine::RenderCallback);

		this->pipeline_manager =
			std::make_shared<Rendering::Vulkan::PipelineManager>(this, this->rendering_engine->GetDeviceManager());

		this->scene_manager->SetSceneLoadPrefix(this->config->scene_path);
		this->scene_manager->LoadScene(this->config->default_scene);
		this->scene_manager->SetScene(this->config->default_scene);

		// Set-up GLFW
		Rendering::GLFWwindowData windowdata = this->rendering_engine->GetWindow();
		glfwSetWindowFocusCallback(windowdata.native_handle, Engine::OnWindowFocusCallback);
		glfwSetCursorPosCallback(windowdata.native_handle, Engine::CursorPositionCallback);
		glfwSetCursorEnterCallback(windowdata.native_handle, Engine::CursorEnterLeaveCallback);
		glfwSetKeyCallback(windowdata.native_handle, Engine::OnKeyEventCallback);
		glfwSetErrorCallback(Engine::ErrorCallback);

		// Fire ON_INIT event
		auto on_init_event	  = EventHandling::Event(this, EventHandling::EventType::ON_INIT);
		int on_init_event_ret = this->FireEvent(on_init_event);

		// Measure and log ON_INIT time
		static constexpr double nano_to_msec = 1.e6;
		double total = static_cast<double>((std::chrono::steady_clock::now() - start).count()) / nano_to_msec;
		this->logger->SimpleLog(
			Logging::LogLevel::Info,
			LOGPFX_CURRENT "Handling ON_INIT took %.3lf ms total and returned %i",
			total,
			on_init_event_ret
		);

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
		glfwSetWindowShouldClose(this->rendering_engine->GetWindow().native_handle, 1);
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
