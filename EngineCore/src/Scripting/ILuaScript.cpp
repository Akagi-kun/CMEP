#include "Scripting/ILuaScript.hpp"

#include "Scripting/Mappings.hpp"

#include "EventHandling.hpp"
#include "lua.h"

#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_LUA_SCRIPT
#include "Logging/LoggingPrefix.hpp" // IWYU pragma: keep

namespace Engine::Scripting
{
	static void ProfilerCallback(void* data, lua_State* state, int samples, int vmstate)
	{
		(void)(samples);

		auto* cast_data = static_cast<ScriptPerfState*>(data);

		size_t dump_len						 = 0;
		static constexpr uint_fast16_t depth = 5;
		const char* stack_dump				 = luaJIT_profile_dumpstack(state, "fZ;", depth, &dump_len);

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
	}

#pragma region Static functions

	std::string UnwindStack(lua_State* of_state)
	{
		std::string error_msg = "\n--- BEGIN LUA STACK UNWIND ---\n\nError that caused this stack unwind:\n";

		std::istringstream caused_by(lua_tostring(of_state, -1));
		lua_pop(of_state, 1);

		std::string line;
		while (std::getline(caused_by, line))
		{
			error_msg.append("\t").append(line).append("\n");
		}
		error_msg.append("\n");

		int caller_level = 0;
		while (true)
		{
			lua_Debug activation_record = {};

			if (lua_getstack(of_state, caller_level, &activation_record) != 1)
			{
				break;
			}

			lua_getinfo(of_state, "nS", &activation_record);

			if (caller_level > 0)
			{
				error_msg.append("called by ");
			}
			else
			{
				error_msg.append("thrown by ");
			}

			error_msg.append(activation_record.source)
				.append(":")
				.append(std::to_string(activation_record.linedefined))
				.append("\n");

			caller_level++;
		}

		if (caller_level == 0)
		{
			error_msg.append("could not unwind stack for this error");
		}

		error_msg.append("\n--- END LUA STACK UNWIND ---\n");

		return error_msg;
	}

	// Register C callback functions from mappings
	static void RegisterCallbacks(lua_State* state, const std::shared_ptr<Logging::Logger>& logger)
	{
		lua_newtable(state);

		for (auto& mapping : Mappings::mappings)
		{
			lua_pushcfunction(state, mapping.second);
			lua_setfield(state, -2, mapping.first.c_str());
		}

		lua_setglobal(state, "engine");

		// Replace print by a SimpleLog wrapper
		void* ptr_obj = lua_newuserdata(state, sizeof(std::weak_ptr<Logging::Logger>));
		new (ptr_obj) std::weak_ptr<Logging::Logger>(logger);

		lua_pushcclosure(state, Mappings::Functions::PrintReplace, 1);
		lua_setglobal(state, "print");

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
			return luaL_error(state, "Exception wrapper caught exception! e.what(): %s", str);
		}
		catch (std::exception& e)
		{
			return luaL_error(state, "Exception wrapper caught exception! e.what(): %s", e.what());
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
		std::string require_origin_str		 = require_origin.remove_filename().parent_path().string();

		using namespace std::literals;
		// Check whether this works on other systems
		std::string require_path_str = (require_origin_str + "/modules/?.lua"s);

		lua_pushstring(state, require_path_str.c_str());
		lua_setfield(state, -2, "path");
	}

	static void RegisterWrapper(lua_State* state)
	{
		lua_pushlightuserdata(state, reinterpret_cast<void*>(LuajitExceptionWrap));
		luaJIT_setmode(state, -1, LUAJIT_MODE_WRAPCFUNC | LUAJIT_MODE_ON);
		lua_pop(state, 1);
	}

	int LuaErrorHandler(lua_State* state)
	{
		// Describe error by unwinding the stack
		lua_pushstring(state, UnwindStack(state).c_str());

		// UnwindStack pops the error string
		// so we return only a single string containing the stack trace
		return 1;
	}

#pragma endregion

	void ILuaScript::LoadAndCompileScript()
	{
		using namespace std::string_literals;

		RegisterRequirePath(state, this);

		std::filesystem::path script_path = path;

		// Load file and compile it
		// this can raise syntax errors
		int load_return = luaL_loadfile(state, script_path.string().c_str());
		if (load_return != LUA_OK)
		{
			throw std::runtime_error("Exception compiling Lua script! loadfile: "s.append(std::to_string(load_return))
										 .append("\n\t"s)
										 .append(lua_tostring(state, -1)));
		}

		// "Interpret" the script
		int pcall_return = lua_pcall(state, 0, LUA_MULTRET, 0);
		if (pcall_return != LUA_OK)
		{
			throw std::runtime_error("Exception compiling Lua script! pcall: "s.append(std::to_string(pcall_return))
										 .append("\n\t")
										 .append(lua_tostring(state, -1)));
		}

		// Uncomment this to disable JIT compiler
		// luaJIT_setmode(state, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_OFF);

		// Register c callback functions
		RegisterCallbacks(state, this->logger);

		// Register exception-handling wrapper
		RegisterWrapper(state);

		this->logger->SimpleLog(
			Logging::LogLevel::Debug1,
			LOGPFX_CURRENT "Loaded and compiled Lua script: '%s'",
			script_path.string().c_str()
		);
	}

#pragma region Public functions

	ILuaScript::ILuaScript(Engine* with_engine, std::filesystem::path with_path, bool with_enable_profiling)
		: InternalEngineObject(with_engine), path(std::move(with_path)), enable_profiling(with_enable_profiling)
	{
		state = luaL_newstate();
		luaL_openlibs(state);

		if (with_enable_profiling)
		{
			luaJIT_profile_start(state, "fl", &ProfilerCallback, &profiler_state);
		}

		LoadAndCompileScript();
	}

	ILuaScript::~ILuaScript()
	{
		if (enable_profiling)
		{
			luaJIT_profile_stop(state);

			this->logger->SimpleLog(
				Logging::LogLevel::Debug2,
				"Profiling result:\n E:%i N:%i I:%i\n",
				profiler_state.engine_count,
				profiler_state.native_count,
				profiler_state.interpreted_count
			);
		}

		lua_close(state);
	}

	int ILuaScript::CallFunction(const std::string& function, void* data)
	{
		// Perform actual call
		int errcall = InternalCall(function, data);

		if (errcall != LUA_OK)
		{
			const char* errormsg = lua_tostring(state, -1);

			// TODO: Throw
			this->logger->SimpleLog(
				Logging::LogLevel::Error,
				LOGPFX_CURRENT "Error when calling Lua\n\tscript '%s'\n\tfunction: "
							   "'%s'\n\terrorcode: %i\n%s",
				path.c_str(),
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
