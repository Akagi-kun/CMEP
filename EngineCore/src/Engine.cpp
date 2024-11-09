#include "Engine.hpp"

#include "Assets/AssetManager.hpp"
#include "Rendering/MeshBuilders/AxisMeshBuilder.hpp"
#include "Rendering/Renderers/Renderer.hpp"
#include "Rendering/Vulkan/backend.hpp"
#include "Rendering/Vulkan/exports.hpp"

#include "Factories/ObjectFactory.hpp"

#include "Logging/Logging.hpp"

#include "EnumStringConvertor.hpp"
#include "EventHandling.hpp"
#include "Exception.hpp"
#include "GLFW/glfw3.h"
#include "SceneManager.hpp"
#include "SceneObject.hpp"
#include "TimeMeasure.hpp"
#include "buildinfo.hpp"
#include "nlohmann/json.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <exception>
#include <fstream>
#include <memory>
#include <queue>
#include <ratio>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

namespace Engine
{
	namespace
	{
		// Utility sleep function
		// algorithm I found somewhere on stackoverflow
		void spinSleep(double seconds)
		{
			// Return if under some small value
			constexpr double bypass_limit = 0.001;
			if (seconds < bypass_limit) { return; }

			constexpr double spin_init = 5e-3;

			double	estimate   = spin_init;
			double	mean	   = spin_init;
			double	sumsquares = 0; // also known as m2
			int64_t count	   = 1;

			while (seconds > estimate)
			{
				// Perform measured sleep
				const auto sleep_start = std::chrono::steady_clock::now();
				std::this_thread::sleep_for(std::chrono::milliseconds(2));
				const auto sleep_end = std::chrono::steady_clock::now();

				// Observed passage of time in seconds
				// should be roughly equal to time specified in sleep_fore
				const double observed =
					std::chrono::duration<double>(sleep_end - sleep_start).count();

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
			const auto spin_start = std::chrono::steady_clock::now();
			while (std::chrono::duration<double>(std::chrono::steady_clock::now() - spin_start)
					   .count() < seconds)
			{}
		}

		void handleMouseInput(
			Engine*							 origin_engine,
			double							 delta_time,
			const Rendering::Vulkan::Window* window
		)
		{
			const auto& screen_size = window->getFramebufferSize();

			// Initial last position is the center of the window
			static glm::vec<2, double> last_pos = {
				static_cast<double>(screen_size.x) / 2,
				static_cast<double>(screen_size.y) / 2
			};

			// Clamp the mouse event movement to some maximum
			// mitigates fast mouse jumps (i.e. when gaining window focus)
			constexpr double clamp_difference = 128;

			// Check whether we have focus and the mouse is in the "content" area
			if (window->status.is_focus && window->status.is_content)
			{
				if ((window->cursor_position.x - last_pos.x) != 0.0 ||
					(window->cursor_position.y - last_pos.y) != 0.0)
				{
					auto event =
						EventHandling::Event(origin_engine, EventHandling::EventType::onMouseMoved);

					event.mouse.x = std::clamp(
						window->cursor_position.x - last_pos.x,
						-clamp_difference,
						clamp_difference
					);
					event.mouse.y = std::clamp(
						window->cursor_position.y - last_pos.y,
						-clamp_difference,
						clamp_difference
					);

					event.delta_time = delta_time;

					// Fire event, throw exception if return code is non-zero
					int event_return = origin_engine->fireEvent(event);
					EXCEPTION_ASSERT(event_return == 0, "Input Event returned non-zero!");

					// Set last position to the current position
					last_pos = window->cursor_position;
				}
			}
		}

		void handleKeyboardInput(
			Engine*										  origin_engine,
			double										  delta_time,
			std::queue<Rendering::Vulkan::KeyboardEvent>& event_queue
		)
		{
			// Handle every event in the queue
			for (; !event_queue.empty(); event_queue.pop())
			{
				auto& input_event = event_queue.front();

				int event_return = 0;

				switch (input_event.type)
				{
					case Rendering::Vulkan::KeyboardEvent::KEY_PRESS:
					case Rendering::Vulkan::KeyboardEvent::KEY_REPEAT:
					{
						auto event =
							EventHandling::Event(origin_engine, EventHandling::EventType::onKeyDown);
						event.keycode	 = static_cast<uint16_t>(input_event.key);
						event.delta_time = delta_time;
						event_return	 = origin_engine->fireEvent(event);
						break;
					}
					case Rendering::Vulkan::KeyboardEvent::KEY_RELEASE:
					{
						auto event =
							EventHandling::Event(origin_engine, EventHandling::EventType::onKeyUp);
						event.keycode	 = static_cast<uint16_t>(input_event.key);
						event.delta_time = delta_time;
						event_return	 = origin_engine->fireEvent(event);
						break;
					}
					default:
					{
						throw ENGINE_EXCEPTION("Unknown input event!");
					}
				}

				// Throw exception on non-zero event return code
				EXCEPTION_ASSERT(event_return == 0, "Input Event returned non-zero!");
			}
		}
	} // namespace

	void Engine::handleInput(const double delta_time)
	{
		auto* window_data = vk_instance->getWindow();

		handleMouseInput(this, delta_time, window_data);

		handleKeyboardInput(this, delta_time, window_data->keyboard_events);
	}

	void Engine::handleConfig()
	{
		std::ifstream file(config_path);

		nlohmann::json data;
		try
		{
			data = nlohmann::json::parse(file);
		}
		catch (...)
		{
			std::throw_with_nested(ENGINE_EXCEPTION("Exception parsing config!"));
		}

		config = {
			.window =
				{
					.size =
						{data["window"]["sizeX"].get<uint16_t>(),
						 data["window"]["sizeY"].get<uint16_t>()},
					.title = data["window"]["title"].get<std::string>(),
				},
			.framerate_target = data["rendering"]["framerateTarget"].get<uint16_t>(),

			.scene_path	 = data["scene_path"].get<std::string>(),
			.shader_path = data["shader_path"].get<std::string>(),

			.default_scene = data["default_scene"].get<std::string>(),

		};
	}

	void Engine::renderCallback(
		Rendering::Vulkan::CommandBuffer* command_buffer,
		uint32_t						  current_frame,
		void*							  engine
	)
	{
		auto* engine_cast	= static_cast<Engine*>(engine);
		auto  current_scene = engine_cast->scene_manager->getSceneCurrent();

		const auto& objects = current_scene->getAllObjects();

		for (const auto& [name, ptr] : objects)
		{
			try
			{
				ptr->getRenderer()->render(command_buffer, current_frame);
			}
			catch (...)
			{
				std::throw_with_nested(ENGINE_EXCEPTION("Caught exception rendering object!"));
			}
		}
	}

	void Engine::engineLoop()
	{
		using dur_second_t = std::chrono::duration<double>;
		using dur_milli_t  = std::chrono::duration<double, std::milli>;
		using namespace std::chrono_literals;

		auto scene = scene_manager->getSceneCurrent();

		/**
		 * @todo Remove this!
		 */
		// Create axis object
		{
			auto* object = Factories::ObjectFactory::createSceneObject<
				Rendering::Renderer3D,
				Rendering::AxisMeshBuilder>(this, "axis", {}, {});

			scene->addObject("_axis", object);
		}

		// Pre-make ON_UPDATE event so we don't have to create it over and over again in hot loop
		auto on_update_event = EventHandling::Event(this, EventHandling::EventType::onUpdate);

		this->logger->logSingle<decltype(this)>(
			Logging::LogLevel::VerboseDebug,
			"Locked to framerate {}{}",
			config.framerate_target,
			config.framerate_target == 0 ? " (VSYNC)" : ""
		);

		// Build scene
		{
			TIMEMEASURE_START(scenebuild);

			const auto& objects = scene->getAllObjects();

			this->logger->logSingle<decltype(this)>(
				Logging::LogLevel::Info,
				"Starting scene build ({} objects)",
				objects.size()
			);
			for (const auto& [name, object] : objects)
			{
				this->logger->logSingle<decltype(this)>(
					Logging::LogLevel::Debug,
					"Building object '{}'",
					name
				);
				object->getMeshBuilder()->build();
			}

			TIMEMEASURE_END_MILLI(scenebuild);

			this->logger->logSingle<decltype(this)>(
				Logging::LogLevel::Info,
				"Scene build finished in {:.3f}ms",
				scenebuild_total.count()
			);
		}

		// Show window
		auto* glfw_window = vk_instance->getWindow();
		glfw_window->setVisibility(true);

		dur_milli_t avg_event{};
		uint64_t	avg_event_count{};

		auto prev_clock = std::chrono::steady_clock::now();

		bool first_frame = true;
		// hot loop
		while (!glfw_window->getShouldClose())
		{
			const auto next_clock = std::chrono::steady_clock::now();

			constexpr dur_second_t min_delta = 0.0001s;
			constexpr dur_second_t max_delta = 10000.0s;
			const auto			   delta_time =
				std::clamp(dur_second_t(next_clock - prev_clock), min_delta, max_delta);
			last_delta_time = delta_time.count();

			// Check return code of FireEvent (events should return non-zero codes as failure)
			if (!first_frame)
			{
				handleInput(delta_time.count());

				on_update_event.delta_time = delta_time.count();
				const auto ret			   = fireEvent(on_update_event);
				if (ret != 0)
				{
					this->logger->logSingle<decltype(this)>(
						Logging::LogLevel::Error,
						"Firing event onUpdate returned {}! Exiting event-loop",
						ret
					);
					break;
				}
			}
			first_frame = false;

			const auto event_end = std::chrono::steady_clock::now();

			// Render
			glfw_window->drawFrame();

			const auto draw_end = std::chrono::steady_clock::now();

			// Sync with glfw event loop
			glfwPollEvents();

			const auto poll_end = std::chrono::steady_clock::now();

			const dur_milli_t event_total = (event_end - next_clock);
			const dur_milli_t draw_total  = (draw_end - event_end);
			const dur_milli_t poll_total  = (poll_end - draw_end);

			const dur_milli_t sum_total = event_total + draw_total + poll_total;

			avg_event += event_total;
			avg_event_count++;

			// Warn if a frame takes too long or is too short
			constexpr dur_milli_t limits[] = {12.0ms, 19.0ms, 8.0ms};
			if (event_total > limits[0] || sum_total > limits[1] || sum_total < limits[2])
			{
				this->logger->logSingle<decltype(this)>(
					Logging::LogLevel::Warning,
					"delta {:.3f} sum {:.3f} (event {:.3f} draw {:.3f} poll {:.3f})",
					dur_milli_t(delta_time).count(),
					sum_total.count(),
					event_total.count(),
					draw_total.count(),
					poll_total.count()
				);
			}

			// Time the frame actually took to render
			const dur_second_t frame_actual = std::chrono::steady_clock::now() - next_clock;
			// Time we want the frame to take
			const double frame_expected		= 1.0 / config.framerate_target;
			const double frame_sleep		= frame_expected - frame_actual.count();
			// If VSYNC is disabled and we need to sleep
			if (config.framerate_target != 0 && frame_sleep > 0)
			{
				spinSleep(frame_sleep);
			}

			prev_clock = next_clock;
		}

		this->logger->logSingle<decltype(this)>(
			Logging::LogLevel::Debug,
			"Closing engine (eventtime avg {:.3})",
			avg_event.count() / static_cast<double>(avg_event_count)
		);
	}

	int Engine::fireEvent(EventHandling::Event event)
	{
		int sum = 0;

		auto current_scene = scene_manager->getSceneCurrent();

		// Get all handlers that match this type
		auto [handlers_begin, handlers_end] =
			current_scene->lua_event_handlers.equal_range(event.event_type);

		for (auto handler = handlers_begin; handler != handlers_end; ++handler)
		{
			try
			{
				// Call the handler
				sum += handler->second(&event);
			}
			catch (const std::exception& e)
			{
				// Get the EventType as a string
				std::string_view event_type_str = EnumStringConvertor(event.event_type);

				this->logger->logSingle<decltype(this)>(
					Logging::LogLevel::Exception,
					"Caught exception trying to fire event '{}'! e.what(): {}",
					event_type_str,
					Base::unrollExceptions(e)
				);
			}
		}

		return sum;
	}

	Engine::Engine(std::shared_ptr<Logging::Logger>& with_logger) : logger(with_logger)
	{}

	Engine::~Engine()
	{
		this->logger->logSingle<decltype(this)>(Logging::LogLevel::Info, "Destructor called");

		scene_manager.reset();

		asset_manager.reset();

		pipeline_manager.reset();

		delete vk_instance;
	}

	void Engine::run()
	{
		TIMEMEASURE_START(run);

		this->logger->mapCurrentThreadToName("engine");

		// Engine info printout
		this->logger->logSingle<decltype(this)>(
			Logging::LogLevel::Info,
			"build info:\n////\nRunning {}\nCompiled by {}\n////",
			buildinfo_build,
			buildinfo_compiledby
		);

		// Load configuration
		try
		{
			handleConfig();
		}
		catch (...)
		{
			std::throw_with_nested(ENGINE_EXCEPTION("Exception parsing config!"));
		}

		asset_manager = std::make_shared<AssetManager>(logger);
		scene_manager = std::make_shared<SceneManager>(this);

		vk_instance = new Rendering::Vulkan::Instance(
			this->logger,
			{
				config.window.size,
				config.window.title,
				{
					{GLFW_VISIBLE, GLFW_FALSE},
					{GLFW_RESIZABLE, GLFW_TRUE},
				},
			}
		);

		vk_instance->getWindow()->setRenderCallback(Engine::renderCallback, this);

		pipeline_manager = std::make_shared<Rendering::Vulkan::PipelineManager>(
			logger,
			vk_instance,
			config.game_path + config.shader_path
		);

		scene_manager->setSceneLoadPrefix(config.game_path + config.scene_path);
		scene_manager->loadScene(config.default_scene);
		scene_manager->setScene(config.default_scene);

		TIMEMEASURE_END_MILLI(run);

		this->logger->logSingle<decltype(this)>(
			Logging::LogLevel::Info,
			"Initialized in {:.3f}ms",
			run_total.count()
		);

		engineLoop();
	}

	[[noreturn]] void Engine::throwTest()
	{
		throw std::runtime_error("BEBEACAC");
	}

	void Engine::stop()
	{
		vk_instance->getWindow()->setShouldClose(true);
	}

	void Engine::configFile(std::string path)
	{
		config_path = std::move(path);
	}
} // namespace Engine
