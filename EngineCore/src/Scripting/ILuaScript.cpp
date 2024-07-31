#include "Scripting/ILuaScript.hpp"

#include "Scripting/Mappings.hpp"

#include "EventHandling.hpp"

#include <filesystem>
#include <stdexcept>
#include <utility>

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_LUA_SCRIPT
#include "Logging/LoggingPrefix.hpp"

namespace Engine::Scripting
{
	/* static void ProfilerCallback(void* data, lua_State* state, int samples, int vmstate)
	{
		auto* cast_data = static_cast<ScriptPerfState*>(data);

		size_t dump_len		   = 0;
		const char* stack_dump = luaJIT_profile_dumpstack(state, "fZ;", 5, &dump_len);

		printf("Stack:\n'");

		for (size_t i = 0; i < dump_len; i++)
		{
			fputc(stack_dump[i], stdout);
		}

		printf("'\n");

		switch (vmstate)
		{
			case 'I':
			{
				cast_data->interpreted_count++;
				break;
			}
			case 'N':
			{
				cast_data->native_count++;
				break;
			}
			case 'C':
			{
				cast_data->engine_count++;

				break;
			}
			default:
			{
				printf("Non-handled state '%c'\n", vmstate);
				break;
			}
		}
	} */

#pragma region Static functions

	// Register C callback functions from mappings
	static void RegisterCallbacks(lua_State* state, const std::shared_ptr<Logging::Logger>& logger)
	{
		lua_newtable(state);

		for (auto& mapping : Mappings::mappings)
		{
			lua_pushcfunction(state, mapping.second);
			lua_setfield(state, -2, mapping.first.c_str());
		}

		/*******************************/
		// Logger table
		lua_newtable(state);

		void* ptr_obj = lua_newuserdata(state, sizeof(std::weak_ptr<Logging::Logger>));
		new (ptr_obj) std::weak_ptr<Logging::Logger>(logger);
		lua_setfield(state, -2, "_smart_ptr");

		lua_pushcfunction(state, Mappings::Functions::MetaLoggerSimpleLog);
		lua_setfield(state, -2, "SimpleLog");
		lua_setfield(state, -2, "logger");
		/*******************************/

		// TODO: Rename
		lua_setglobal(state, "engine");

		lua_settop(state, 0);
	}

	static int LuajitExceptionWrap(lua_State* state, lua_CFunction func)
	{
		try
		{
			return func(state);
		}
		catch (const char* str)
		{
			return luaL_error(state, str);
		}
		catch (std::exception& e)
		{
			return luaL_error(state, "Wrapper caught exception! e.what(): %s", e.what());
		}
		catch (...)
		{
			return luaL_error(state, "Unknown error caught");
		}
	}

	static void RegisterRequirePath(lua_State* state, ILuaScript* with_script)
	{
		lua_getglobal(state, LUA_LOADLIBNAME);
		if (!lua_istable(state, -1))
		{
			throw std::runtime_error("Lua Module 'package' is not loaded!");
		}

		std::filesystem::path require_origin = with_script->GetPath();
		std::string require_origin_str		 = require_origin.remove_filename().string();

		using namespace std::literals;
		// Check whether this works on other systems
		std::string require_path_str = (require_origin_str + "/?.lua;"s + require_origin_str + "/modules/?.lua"s);

		lua_pushstring(state, require_path_str.c_str());
		lua_setfield(state, -2, "path");
	}

	static void RegisterWrapper(lua_State* state)
	{
		lua_pushlightuserdata(state, reinterpret_cast<void*>(LuajitExceptionWrap));
		luaJIT_setmode(state, -1, LUAJIT_MODE_WRAPCFUNC | LUAJIT_MODE_ON);
		lua_pop(state, 1);
	}

	static int LuaErrorHandler(lua_State* state)
	{
		// We can handle errors here if necessary
		//
		// last value on stack is the error object
		//
		// last value on stack when returning has to
		// be an error object - original or another
		//
		(void)(state);

		// Simply pass the error object through to pcall
		return 1;
	}

#pragma endregion

	int ILuaScript::LoadAndCompileScript()
	{
		RegisterRequirePath(state, this);

		std::filesystem::path script_path = this->path;

		// Load file and compile it
		int errload			  = luaL_loadfile(state, script_path.string().c_str());
		int errexec			  = lua_pcall(state, 0, LUA_MULTRET, 0);
		const char* errorexec = lua_tostring(state, -1);

		// Uncomment this to disable JIT compiler
		// luaJIT_setmode(state, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_OFF);

		// Register c callback functions
		RegisterCallbacks(state, this->logger);

		// Register exception-handling wrapper
		RegisterWrapper(state);

		if (errload != LUA_OK || errexec != LUA_OK)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Error,
				LOGPFX_CURRENT "Error when loading and compiling Lua script "
							   "'%s'\n   Error codes:\n    load: %i\n    compile: "
							   "%i\n  Compilation error: %s",
				script_path.string().c_str(),
				errload,
				errexec,
				errorexec
			);

			return 3;
		}

		this->logger->SimpleLog(
			Logging::LogLevel::Debug1,
			LOGPFX_CURRENT "Loaded and compiled Lua script: '%s'",
			script_path.string().c_str()
		);

		return 0;
	}

#pragma region Public functions

	ILuaScript::ILuaScript(Engine* with_engine, /* ILuaScript* executor, */ std::string with_path)
		: InternalEngineObject(with_engine), path(std::move(with_path))
	{
		this->state = luaL_newstate();
		luaL_openlibs(this->state);

		this->profiler_state = new ScriptPerfState();

		// luaJIT_profile_start(this->state, "fl", &ProfilerCallback, this->profiler_state);

		int return_code = this->LoadAndCompileScript();
		if (return_code != 0)
		{
			throw std::runtime_error(
				"Could not compile script '" + this->path + "' (errcode = " + std::to_string(return_code) + ")"
			);
		}
	}

	ILuaScript::~ILuaScript()
	{
		/*
		luaJIT_profile_stop(this->state);

		printf(
			"Profiling result:\n E:%i N:%i I:%i\n",
			this->profiler_state->engine_count,
			this->profiler_state->native_count,
			this->profiler_state->interpreted_count
		);
		*/

		delete this->profiler_state;

		lua_close(this->state);
	}

	void ILuaScript::InitializeCall(const std::string& function)
	{
		// Clear stack
		lua_settop(state, 0);

		// Push error handler
		lua_pushcfunction(state, LuaErrorHandler);

		// Get start function
		lua_getglobal(state, function.c_str());
	}

	int ILuaScript::CallFunction(const std::string& function, void* data)
	{
		// Perform actual call
		int errcall = this->InternalCall(function, data);

		if (errcall != LUA_OK)
		{
			const char* errormsg = "";
			errormsg			 = lua_tostring(state, -1);

			this->logger->SimpleLog(
				Logging::LogLevel::Warning,
				LOGPFX_CURRENT "Error when calling Lua\n\tscript '%s'\n\tfunction: "
							   "'%s'\n\terrorcode: %i\n\terrormsg: '%s'",
				this->path.c_str(),
				function.c_str(),
				errcall,
				errormsg
			);

			return errcall;
		}

		lua_Integer ret = lua_tointeger(state, -1);

		return static_cast<int>(ret);
	}
} // namespace Engine::Scripting
