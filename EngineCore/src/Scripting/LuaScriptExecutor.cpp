#include "Scripting/LuaScriptExecutor.hpp"
#include "GlobalSceneManager.hpp"
#include "Scripting/Mappings.hpp"
#include "Logging/Logging.hpp"
#include "AssetManager.hpp"
#include "Engine.hpp"

namespace Engine
{
	namespace Scripting
	{
		// Register C callback functions from mappings
		void LuaScriptExecutor::registerCallbacks(lua_State* state)
		{
			lua_newtable(state);

			for (int i = 0; i < Mappings::countMappings; i++)
			{
				lua_pushcfunction(state, Mappings::functionMappings[i]);
				lua_setfield(state, -2, Mappings::nameMappings[i]);
			}

			lua_setglobal(state, "cmepapi");
		}

		void LuaScriptExecutor::ConfigScriptLoad(LuaScript* script)
		{
			// Our preload settings
			std::vector<std::tuple<std::string, std::string, std::string>> settings;

			std::string current[3];

			// Get script state
			lua_State* state = script->GetState();

			lua_getglobal(state, "config"); // Get settings table

			size_t len = lua_rawlen(state, -1);
			for (int index = 0; index <= len; index++) {

				// Our actual index will be +1 because Lua 1 indexes tables.
				int actualIndex = index + 1;

				// Push our target index to the stack.
				lua_pushinteger(state, actualIndex);

				// Get the table data at this index (and not get the table, which is what I thought this did.)
				lua_gettable(state, -2);

				// Check for the sentinel nil element.
				if (lua_type(state, -1) == LUA_TNIL) break;

				// Get it's value.
				size_t len2 = lua_rawlen(state, -1);
				for (int index2 = 0; index2 < 3; index2++) {

					// Our actual index will be +1 because Lua 1 indexes tables.
					int actualIndex2 = index2 + 1;

					// Push our target index to the stack.
					lua_pushinteger(state, actualIndex2);

					// Get the table data at this index (and not get the table, which is what I thought this did.)
					lua_gettable(state, -2);

					// Check for the sentinel nil element.
					if (lua_type(state, -1) == LUA_TNIL)
					{
						break;
					}
					// Get it's value.
					current[index2] = lua_tostring(state, -1);

					// Pop it off again.

					lua_pop(state, 1);
				}
				settings.push_back(std::make_tuple(current[0], current[1], current[2]));

				lua_pop(state, -2);
			}

			for (auto setting : settings)
			{
				std::string setting_full = std::get<0>(setting);

				std::string setting_class = setting_full.substr(0, setting_full.find("."));
				setting_full.erase(0, setting_full.find(".") + std::string(".").length());
				std::string setting_name = setting_full.substr(0, setting_full.find("."));
				
				//printf("Setting: class '%s' name '%s' with vals '%s', '%s'\n", setting_class.c_str(), setting_name.c_str(), std::get<1>(setting).c_str(), std::get<2>(setting).c_str());

				if (setting_class == "eventHandler")
				{
					::Engine::EventHandling::EventType event_type;

					if (setting_name.compare("onInit") == 0)
					{
						event_type = ::Engine::EventHandling::EventType::ON_INIT;
					}
					else if (setting_name.compare("onUpdate") == 0)
					{
						event_type = ::Engine::EventHandling::EventType::ON_UPDATE;
					}
					else if (setting_name.compare("onKeyDown") == 0)
					{
						event_type = ::Engine::EventHandling::EventType::ON_KEYDOWN;
					}
					else if (setting_name.compare("onMouseMoved") == 0)
					{
						event_type = ::Engine::EventHandling::EventType::ON_MOUSEMOVED;
					}

					AssetManager* asset_manager = ::Engine::global_engine->GetAssetManager();
					if (asset_manager->GetLuaScript(std::get<1>(setting)) == nullptr)
					{
						asset_manager->AddLuaScript(std::get<1>(setting), std::get<1>(setting));
					}

					Scripting::LuaScript* event_handler = asset_manager->GetLuaScript(std::get<1>(setting));
					global_engine->RegisterLuaEventHandler(event_type, event_handler, std::get<2>(setting));

					Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug1, "Event '%s' (%u) is handling file '%s' function '%s'", setting_name.c_str(), event_type, std::get<1>(setting).c_str(), std::get<2>(setting).c_str());
				}
			}
		}

		void LuaScriptExecutor::CallIntoScript(ExecuteType etype, LuaScript* script, std::string function, void* data)
		{
			// Get script state
			lua_State* state = script->GetState();

			// Run the start function in a way decided by the ExecuteType
			lua_getglobal(state, function.c_str()); // Get start function
			int errcall = LUA_OK;
			switch (etype)
			{
				case ExecuteType::EventHandler:
					EventHandling::Event* event = (EventHandling::Event*)data;

					lua_newtable(state);
					lua_pushnumber(state, event->deltaTime);
					lua_setfield(state, -2, "deltaTime");
					lua_pushinteger(state, event->keycode);
					lua_setfield(state, -2, "keycode");
					lua_newtable(state);
					lua_pushnumber(state, event->mouse.x);
					lua_setfield(state, -2, "x");
					lua_pushnumber(state, event->mouse.y);
					lua_setfield(state, -2, "y");
					lua_setfield(state, -2, "mouse");

					errcall = lua_pcall(state, 1, 1, 0); // Call
					break;
			}

			if (errcall != LUA_OK)
			{
				const char* errormsg = "";
				errormsg = lua_tostring(state, -1);

				Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Warning,
					"Error when calling Lua script '%s'\n  Called function: '%s'\n  Call error code: %i\n  Error message: %s",
					script->path.c_str(), function.c_str(), errcall, errormsg);
			}

			int64_t ret = lua_tointeger(state, -1);
			lua_pop(state, 1);

			//Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2,
			//	"Running lua script '%s', called function '%d' returned %llu",
			//	script->path.c_str(), function.c_str(), ret);
		}

		int LuaScriptExecutor::LoadAndCompileScript(LuaScript* script)
		{
			// Get script state
			lua_State* state = script->GetState();

			// Load file and compile it
			int errload = luaL_loadfile(state, script->path.c_str());
			int errexec = lua_pcall(state, 0, LUA_MULTRET, 0);
			const char* errorexec = lua_tostring(state, -1);

			// Register c callback functions
			LuaScriptExecutor::registerCallbacks(state);

			if (errload != LUA_OK || errexec != LUA_OK)
			{
				printf("Error when loading and compiling Lua script '%s'\n   Error codes:\n    load: %i\n    compile: %i\n  Compilation error: %s\n", script->path.c_str(), errload, errexec, errorexec);
			}

			printf("Loaded and compiled Lua script: '%s'\n", script->path.c_str());

			return 0;
		}
	}
}