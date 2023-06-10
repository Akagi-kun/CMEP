#pragma once

#include <unordered_map>
#include <functional>
#include <vector>
#include <memory>

#include "glew/glew.h"
#include "glfw/glfw3.h"

#include "GlobalSceneManager.hpp"
#include "Logging/Logging.hpp"
#include "EventHandling.hpp"

#pragma comment (lib, "glfw3.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "opengl32.lib")

namespace Engine
{
	class AssetManager;
	class ImageObject;
	class Object;
	namespace Rendering { class Window; }

	namespace Scripting
	{
		class LuaScriptExecutor;
		class LuaScript;
	}

	class __declspec(dllexport) Engine final
	{
	private:
		// Keeps internal loop running
		bool run_threads = true;

		std::string config_path;

		// Window
		GLFWwindow* window = nullptr;
		unsigned int windowX = 0, windowY = 0;
		std::string windowTitle;
		unsigned int framerateTarget = 30;

		// Managers
		AssetManager* asset_manager = nullptr;
		Scripting::LuaScriptExecutor* script_executor = nullptr;

		std::vector<std::pair<EventHandling::EventType, std::function<void(EventHandling::Event&)>>> event_handlers;
		std::vector<std::tuple<EventHandling::EventType, Scripting::LuaScript*, std::string>> lua_event_handlers;

		static void APIENTRY debugCallbackGL(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) noexcept;

		static void spinSleep(double seconds);

		void handleInput(const double deltaTime) noexcept;

		void engineLoop();

		void HandleConfig();

		void FireEvent(EventHandling::Event& event);
	public:
		Engine(const char* windowTitle, const unsigned windowX, const unsigned windowY) noexcept;
		~Engine() noexcept;

		void SetFramerateTarget(unsigned framerate) noexcept;

		void Init();
		void Run();

		void ConfigFile(std::string path);
		void RegisterEventHandler(EventHandling::EventType event_type, std::function<void(EventHandling::Event&)> function);
		void RegisterLuaEventHandler(EventHandling::EventType event_type, Scripting::LuaScript* script, std::string function);

		void AddObject(std::string name, Object* ptr);
		Object* FindObject(std::string name);
		size_t RemoveObject(std::string name) noexcept;
		
		AssetManager* GetAssetManager() noexcept;
	};

	__declspec(dllexport) Engine* initializeEngine(const char* windowTitle, const unsigned windowX, const unsigned windowY);

	extern __declspec(dllexport) Engine* global_engine;
}