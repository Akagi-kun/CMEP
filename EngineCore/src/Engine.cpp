#include "Engine.hpp"

#include "Assets/AssetManager.hpp"
#include "Rendering/MeshBuilders/AxisMeshBuilder.hpp"
#include "Rendering/Renderers/Renderer.hpp"
#include "Rendering/Vulkan/backend.hpp"
#include "Rendering/Vulkan/exports.hpp"

#include "Factories/ObjectFactory.hpp"

#include "Logging/Logging.hpp"

#include "GLFW/glfw3.h"
#include "Object.hpp"
#include "buildinfo.hpp"
#include "nlohmann/json.hpp"

#include <chrono>
#include <cstdint>
#include <exception>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <thread>

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_ENGINE
#include "Logging/LoggingPrefix.hpp" // IWYU pragma: keep

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

	void Engine::HandleInput(const double delta_time)
	{
		auto* window_data		= vk_instance->GetWindow();
		const auto& screen_size = window_data->GetFramebufferSize();

		static glm::vec<2, double>
			/* static Vector2<double> */
			last_pos = {static_cast<double>(screen_size.x) / 2, static_cast<double>(screen_size.y) / 2};

		static constexpr double clamp_difference = 128;

		if (window_data->status.is_focus && window_data->status.is_content)
		{
			if ((window_data->cursor_position.x - last_pos.x) != 0.0 ||
				(window_data->cursor_position.y - last_pos.y) != 0.0)
			{
				auto event = EventHandling::Event(this, EventHandling::EventType::ON_MOUSEMOVED);

				event.mouse.x =
					std::clamp(window_data->cursor_position.x - last_pos.x, -clamp_difference, clamp_difference);
				event.mouse.y =
					std::clamp(window_data->cursor_position.y - last_pos.y, -clamp_difference, clamp_difference);

				event.delta_time = delta_time;
				FireEvent(event);

				last_pos = window_data->cursor_position;
			}
		}

		while (!window_data->input_events.empty())
		{
			auto& input_event = window_data->input_events.front();
			// begín

			switch (input_event.type)
			{
				case Rendering::Vulkan::InputEvent::KEY_PRESS:
				case Rendering::Vulkan::InputEvent::KEY_REPEAT:
				{
					auto event		 = EventHandling::Event(this, EventHandling::EventType::ON_KEYDOWN);
					event.keycode	 = static_cast<uint16_t>(input_event.key);
					event.delta_time = GetLastDeltaTime();
					FireEvent(event);
					break;
				}
				case Rendering::Vulkan::InputEvent::KEY_RELEASE:
				{

					auto event		 = EventHandling::Event(this, EventHandling::EventType::ON_KEYUP);
					event.keycode	 = static_cast<uint16_t>(input_event.key);
					event.delta_time = GetLastDeltaTime();
					FireEvent(event);
					break;
				}
				default:
				{
					throw std::invalid_argument("Unknown input event!");
				}
			}

			// end
			window_data->input_events.pop();
		}
	}

	void Engine::HandleConfig()
	{
		std::ifstream file(config_path);

		config = std::make_unique<EngineConfig>();

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
				config_path.c_str(),
				e.what()
			);
			exit(1);
		}

		config->window.title  = data["window"]["title"].get<std::string>();
		config->window.size.x = data["window"]["sizeX"].get<uint16_t>();
		config->window.size.y = data["window"]["sizeY"].get<uint16_t>();

		config->framerate_target = data["rendering"]["framerateTarget"].get<uint16_t>();

		config->scene_path	  = data["scene_path"].get<std::string>();
		config->default_scene = data["default_scene"].get<std::string>();

		config->shader_path = data["shader_path"].get<std::string>();
	}

	void Engine::RenderCallback(Rendering::Vulkan::CommandBuffer* command_buffer, uint32_t current_frame, void* engine)
	{
		auto* engine_cast = static_cast<Engine*>(engine);

		auto& current_scene = engine_cast->scene_manager->GetSceneCurrent();

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
				engine_cast->logger->SimpleLog(
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

		auto& scene = scene_manager->GetSceneCurrent();

		// TODO: Remove this!
		// Create axis object
		{
			auto* object =
				Factories::ObjectFactory::CreateSceneObject<Rendering::Renderer3D, Rendering::AxisMeshBuilder>(
					this,
					"axis",
					{}
				);

			scene->AddObject("_axis", object);
		}

		// Pre-make ON_UPDATE event so we don't have to create it over and over again in hot loop
		auto on_update_event = EventHandling::Event(this, EventHandling::EventType::ON_UPDATE);

		this->logger->SimpleLog(
			Logging::LogLevel::Debug1,
			LOGPFX_CURRENT "Locked to framerate %u%s",
			config->framerate_target,
			config->framerate_target == 0 ? " (VSYNC)" : ""
		);

		auto build_clock = std::chrono::steady_clock::now();

		this->logger->SimpleLog(Logging::LogLevel::Info, "Starting scene build");
		for (const auto& [name, object] : scene->GetAllObjects())
		{
			this->logger->SimpleLog(Logging::LogLevel::Debug1, LOGPFX_CURRENT "Building object '%s'", name.c_str());
			object->GetRenderer()->ForceBuild();
		}

		auto scene_build_time = static_cast<double>((std::chrono::steady_clock::now() - build_clock).count()) /
								nano_to_msec;

		this->logger->SimpleLog(
			Logging::LogLevel::Info,
			LOGPFX_CURRENT "Scene build finished in %lums",
			static_cast<uint_fast32_t>(scene_build_time)
		);

		// Show window
		auto* glfw_window = vk_instance->GetWindow();
		// glfwShowWindow(glfw_window->native_handle);
		glfw_window->SetVisibility(true);

		double average_event_total	 = 0.00;
		uint64_t average_event_count = 1;

		auto prev_clock = std::chrono::steady_clock::now();

		bool first_frame = true;
		// hot loop
		while (!glfw_window->GetShouldClose())
		{
			const auto next_clock	= std::chrono::steady_clock::now();
			static constexpr double min_delta = 0.1f / sec_to_msec;
			static constexpr double max_delta = 100000.f;
			const double delta_time = std::clamp(static_cast<double>((next_clock - prev_clock).count()) / nano_to_sec, min_delta, max_delta);
			last_delta_time			= delta_time;

			// Check return code of FireEvent (events should return non-zero codes as failure)
			if (!first_frame)
			{
				HandleInput(delta_time);

				on_update_event.delta_time = delta_time;
				const auto ret			   = FireEvent(on_update_event);
				if (ret != 0)
				{
					this->logger->SimpleLog(
						Logging::LogLevel::Error,
						LOGPFX_CURRENT "Firing event ON_UPDATE returned %u! Exiting event-loop",
						ret
					);
					break;
				}
			}
			first_frame = false;

			const auto event_clock = std::chrono::steady_clock::now();

			// Render
			glfw_window->DrawFrame();

			const auto draw_clock = std::chrono::steady_clock::now();

			// Sync with glfw event loop
			glfwPollEvents();

			const auto poll_clock = std::chrono::steady_clock::now();

			const double event_time = static_cast<double>((event_clock - next_clock).count()) / nano_to_msec;
			const double draw_time	= static_cast<double>((draw_clock - event_clock).count()) / nano_to_msec;
			const double poll_time	= static_cast<double>((poll_clock - draw_clock).count()) / nano_to_msec;

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
			const double sleep_secs = 1.0 / framerate_target -
									  static_cast<double>((frame_clock - next_clock).count()) / nano_to_sec;
			// spin sleep if sleep necessary and VSYNC disabled
			if (sleep_secs > 0 && framerate_target != 0)
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

		auto current_scene	   = scene_manager->GetSceneCurrent();
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
					LOGPFX_CURRENT "Caught exception trying to fire event '%u'! e.what(): %s",
					event.event_type,
					e.what()
				);
			}
		}

		return sum;
	}

	double Engine::GetLastDeltaTime() const
	{
		return last_delta_time;
	}

	Engine::Engine(std::shared_ptr<Logging::Logger>& with_logger) : logger(with_logger)
	{
	}

	Engine::~Engine()
	{
		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Destructor called");

		scene_manager.reset();

		asset_manager.reset();

		pipeline_manager.reset();

		delete vk_instance;
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
			HandleConfig();
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

		asset_manager = std::make_shared<AssetManager>(this);

		scene_manager = std::make_shared<SceneManager>(this);
	}

	void Engine::Run()
	{
		auto start = std::chrono::steady_clock::now();

		vk_instance = new Rendering::Vulkan::Instance(
			this->logger,
			{
				config->window.size,
				config->window.title,
				{
					{GLFW_VISIBLE, GLFW_FALSE},
					{GLFW_RESIZABLE, GLFW_TRUE},
				},
			}
		);

		vk_instance->GetWindow()->SetRenderCallback(Engine::RenderCallback, this);

		pipeline_manager =
			std::make_shared<Rendering::Vulkan::PipelineManager>(logger, vk_instance, config->shader_path);

		scene_manager->SetSceneLoadPrefix(config->scene_path);
		scene_manager->LoadScene(config->default_scene);
		scene_manager->SetScene(config->default_scene);

		// Fire ON_INIT event
		auto on_init_event	  = EventHandling::Event(this, EventHandling::EventType::ON_INIT);
		int on_init_event_ret = FireEvent(on_init_event);

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

		EngineLoop();
	}

	[[noreturn]] void Engine::ThrowTest()
	{
		throw std::runtime_error("BEBEACAC");
	}

	void Engine::Stop()
	{
		vk_instance->GetWindow()->SetShouldClose(true);
	}

	void Engine::ConfigFile(std::string path)
	{
		config_path = std::move(path);
	}
} // namespace Engine
