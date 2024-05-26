#include "Scripting/LuaScriptExecutor.hpp"
#include "SceneManager.hpp"
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

			for(auto& mapping : Mappings::mappings)
			{
				lua_pushcfunction(state, mapping.second);
				lua_setfield(state, -2, mapping.first.c_str());
			}

			lua_setglobal(state, "cmepapi");
		}

		// Register meta information (logger etc)
		// this information can be used in C callbacks
		void LuaScriptExecutor::registerMeta(lua_State* state)
		{
			// cmepmeta table (not an actual Lua metatable !!!)
			lua_newtable(state);

			/*******************************/
			// Logger table
			lua_newtable(state);

			void* ptr_obj = lua_newuserdata(state, sizeof(std::weak_ptr<Logging::Logger>));
			new(ptr_obj) std::weak_ptr<Logging::Logger>(this->logger);
			
			lua_setfield(state, -2, "_smart_pointer");

			lua_pushcfunction(state, Mappings::Functions::metaLogger_SimpleLog);
			lua_setfield(state, -2, "SimpleLog");

			lua_setfield(state, -2, "logger");
			/*******************************/
			/*******************************/

			lua_setglobal(state, "cmepmeta");
		}

		static int LuaErrorHandler(lua_State* state)
		{
			// We can handle errors here if necessary
			//
			// last value on stack is the error object
			//
			// last value on stack when returning has to
			// be an error object original or another
			//
			(void)(state);

			// Simply pass the error object through to pcall
			return 1;
		}

		int LuaScriptExecutor::CallIntoScript(ExecuteType etype, std::shared_ptr<LuaScript> script, std::string function, void* data)
		{
			//this->logger->SimpleLog(Logging::LogLevel::Debug2,
			//	"Running lua script '%s', called function '%s'",
			//	script->path.c_str(), function.c_str());

			// Get script state
			lua_State* state = script->GetState();

			// Run the start function in a way decided by the ExecuteType
			lua_pushcfunction(state, LuaErrorHandler);
			lua_getglobal(state, function.c_str()); // Get start function
			int errcall = LUA_OK;
			switch (etype)
			{
				case ExecuteType::EventHandler:
					EventHandling::Event* event = (EventHandling::Event*)data;

					// Event table
					lua_newtable(state);
					lua_pushnumber(state, event->deltaTime);
					lua_setfield(state, -2, "deltaTime");

					lua_pushinteger(state, event->keycode);
					lua_setfield(state, -2, "keycode");

					Engine** engine = (Engine**)lua_newuserdata(state, sizeof(Engine*));
					*engine = event->raisedFrom;
					lua_setfield(state, -2, "engine");
					
					// Mouse table
					lua_newtable(state);
					lua_pushnumber(state, event->mouse.x);
					lua_setfield(state, -2, "x");
					lua_pushnumber(state, event->mouse.y);
					lua_setfield(state, -2, "y");
					lua_setfield(state, -2, "mouse");

					// Call into script
					errcall = lua_pcall(state, 1, 1, -3); // Call
					break;
			}

			if (errcall != LUA_OK)
			{
				const char* errormsg = "";
				errormsg = lua_tostring(state, -1);

				this->logger->SimpleLog(Logging::LogLevel::Warning,
					"Error when calling Lua\n\tscript '%s' function: '%s'\n\tCall error code: %i\n\tError message: %s",
					script->path.c_str(), function.c_str(), errcall, errormsg);
			}

			lua_Integer ret = lua_tointeger(state, -1);
			lua_pop(state, 1);

			return static_cast<int>(ret);
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
			this->registerMeta(state);

			if (errload != LUA_OK || errexec != LUA_OK)
			{
				this->logger->SimpleLog(Logging::LogLevel::Error, "Error when loading and compiling Lua script '%s'\n   Error codes:\n    load: %i\n    compile: %i\n  Compilation error: %s", script->path.c_str(), errload, errexec, errorexec);
			
				return 1;
			}

			this->logger->SimpleLog(Logging::LogLevel::Debug1, "Loaded and compiled Lua script: '%s'", script->path.c_str());

			return 0;
		}
	}
}