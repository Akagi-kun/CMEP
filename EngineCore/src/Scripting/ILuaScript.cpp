#include "Scripting/ILuaScript.hpp"

#include "Scripting/API/Global_API.hpp"
#include "Scripting/Utility.hpp"

#include "Logging/Logging.hpp"

#include "EventHandling.hpp"
#include "Exception.hpp"
#include "InternalEngineObject.hpp"
#include "lua.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <format>
#include <memory>
#include <string>
#include <utility>

namespace Engine::Scripting
{
#pragma region Static

	namespace
	{
		void profilerCallback(void* data, lua_State* state, int samples, int vmstate)
		{
			(void)(samples);

			auto* cast_data = static_cast<ScriptPerfState*>(data);

			size_t						   dump_len = 0;
			static constexpr uint_fast16_t depth	= 5;
			const char* stack_dump = luaJIT_profile_dumpstack(state, "fZ;", depth, &dump_len);

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

		// Register C callback functions from mappings
		void registerCallbacks(lua_State* state, const std::shared_ptr<Logging::Logger>& logger)
		{
			// Push global mappings into global namespace
			for (auto& mapping : API::global_mappings)
			{
				lua_pushcfunction(state, mapping.second);
				lua_setglobal(state, mapping.first.c_str());
			}

			// Replace print by a logger wrapper
			void* ptr_obj = lua_newuserdata(state, sizeof(std::weak_ptr<Logging::Logger>));
			new (ptr_obj) std::weak_ptr<Logging::Logger>(logger);

			lua_pushcclosure(state, API::printReplace, 1);
			lua_setglobal(state, "print");

			lua_settop(state, 0);
		}

		// Catches exceptions when executing lua-mapped C/C++ code and returns them as
		// luaL_error functions can still return them manually,
		int luajitExceptionWrap(lua_State* state, lua_CFunction function)
		{
			try
			{
				return function(state);
			}
			catch (const char* str)
			{
				return luaL_error(state, "Exception wrapper caught exception! e.what():\n%s", str);
			}
			catch (const std::exception& e)
			{
				return luaL_error(
					state,
					"Exception wrapper caught exception!\n\tFunction: %s\n\tStacktrace: "
					"%s",
					Utility::mappingReverseLookup(function).data(),
					Base::unrollExceptions(e).c_str()
				);
			}
			catch (...)
			{
				return luaL_error(state, "Exception wrapper caught unknown exception!");
			}
		}

		void registerRequirePath(lua_State* state, ILuaScript* with_script)
		{
			lua_getglobal(state, LUA_LOADLIBNAME);
			EXCEPTION_ASSERT(lua_istable(state, -1), "Lua Module 'package' is not loaded!");

			/** @todo Take require path from somewhere else, scripts aren't guaranteed to be in the `./scripts/` directory. */
			std::filesystem::path require_origin = with_script->getPath();
			std::string require_origin_str = require_origin.remove_filename().parent_path().string();

			std::string require_path_str = (require_origin_str + "/modules/?.lua");

			lua_pushstring(state, require_path_str.c_str());
			lua_setfield(state, -2, "path");
		}

		void registerWrapper(lua_State* state)
		{
			lua_pushlightuserdata(state, reinterpret_cast<void*>(luajitExceptionWrap));
			luaJIT_setmode(state, -1, LUAJIT_MODE_WRAPCFUNC | LUAJIT_MODE_ON);
			lua_pop(state, 1);
		}

	} // namespace

#pragma endregion

	void ILuaScript::loadAndCompileScript()
	{
		registerRequirePath(state, this);

		std::filesystem::path script_path = path;

		// Register exception-handling wrapper
		registerWrapper(state);

		// Load file and compile it
		// this can raise syntax errors
		int load_return = luaL_loadfile(state, script_path.string().c_str());
		if (load_return != LUA_OK)
		{
			throw ENGINE_EXCEPTION(std::format(
				"Exception loading Lua script! loadfile: {}\n\t{}",
				load_return,
				lua_tostring(state, -1)
			));
		}

		// "Interpret" the script
		int pcall_return = lua_pcall(state, 0, LUA_MULTRET, 0);
		if (pcall_return != LUA_OK)
		{
			throw ENGINE_EXCEPTION(std::format(
				"Exception compiling Lua script! pcall: {}\n\t{}",
				pcall_return,
				lua_tostring(state, -1)
			));
		}

		// Uncomment this to disable JIT compiler
		// luaJIT_setmode(state, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_OFF);

		// Register c callback functions
		registerCallbacks(state, this->logger);

		this->logger->logSingle<decltype(this)>(
			Logging::LogLevel::Debug,
			"Loaded and compiled Lua script: '{}'",
			path.lexically_normal().string()
		);
	}

#pragma endregion

#pragma region Public

	ILuaScript::ILuaScript(
		Engine*				  with_engine,
		std::filesystem::path with_path,
		bool				  with_enable_profiling
	)
		: InternalEngineObject(with_engine), path(std::move(with_path)),
		  enable_profiling(with_enable_profiling)
	{
		state = luaL_newstate();
		luaL_openlibs(state);

		if (with_enable_profiling)
		{
			luaJIT_profile_start(state, "fl", &profilerCallback, &profiler_state);
		}

		loadAndCompileScript();
	}

	ILuaScript::~ILuaScript()
	{
		if (enable_profiling)
		{
			luaJIT_profile_stop(state);
		}

		lua_close(state);
	}

	int ILuaScript::callFunction(const std::string& function, void* data)
	{
		// Perform actual call
		int errcall = internalCall(function, data);

		if (errcall != LUA_OK)
		{
			const char* errormsg = lua_tostring(state, -1);

			throw ENGINE_EXCEPTION(std::format(
				"Error '{}' occured calling script '{}' (fn: '{}')\n{}",
				errcall,
				path.lexically_normal().string(),
				function,
				errormsg
			));
		}

		lua_Integer ret = lua_tointeger(state, -1);

		return static_cast<int>(ret);
	}

	// NOLINTNEXTLINE(readability-make-member-function-const)
	int ScriptFunctionRef::operator()(void* data)
	{
		auto locked_script = script.lock();
		EXCEPTION_ASSERT(locked_script, "Could not lock script!");

		return locked_script->callFunction(function, data);
	}
} // namespace Engine::Scripting
