#include "Scripting/ILuaScript.hpp"

#include "Scripting/Mappings.hpp"
#include "Scripting/Utility.hpp"

#include "EventHandling.hpp"
#include "Exception.hpp"
#include "lua.hpp"

#include <filesystem>
#include <string>
#include <utility>

namespace Engine::Scripting
{
#pragma region Static functions

	namespace
	{
		void ProfilerCallback(void* data, lua_State* state, int samples, int vmstate)
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
		void RegisterCallbacks(lua_State* state, const std::shared_ptr<Logging::Logger>& logger)
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

		int LuajitExceptionWrap(lua_State* state, lua_CFunction function)
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
					"Exception wrapper caught exception!\nFunction: %s\nStacktrace: %s",
					Utility::MappingReverseLookup(function).data(),
					UnrollExceptions(e).c_str()
				);
			}
			catch (...)
			{
				return luaL_error(state, "Exception wrapper caught unknown exception!");
			}
		}

		void RegisterRequirePath(lua_State* state, ILuaScript* with_script)
		{
			lua_getglobal(state, LUA_LOADLIBNAME);
			ENGINE_EXCEPTION_ON_ASSERT(
				lua_istable(state, -1),
				"Lua Module 'package' is not loaded!"
			)

			std::filesystem::path require_origin = with_script->GetPath();
			std::string require_origin_str = require_origin.remove_filename().parent_path().string(
			);

			using namespace std::string_literals;
			std::string require_path_str = (require_origin_str + "/modules/?.lua"s);

			lua_pushstring(state, require_path_str.c_str());
			lua_setfield(state, -2, "path");
		}

		void RegisterWrapper(lua_State* state)
		{
			lua_pushlightuserdata(state, reinterpret_cast<void*>(LuajitExceptionWrap));
			luaJIT_setmode(state, -1, LUAJIT_MODE_WRAPCFUNC | LUAJIT_MODE_ON);
			lua_pop(state, 1);
		}

		int CdefLazyLoader(lua_State* state)
		{
			const char* preload_code =
				"local ffi = require('ffi');"
				"local lib = ffi.load('EngineCore', true)"
				"ffi.cdef[[\n"
				"void* CreateGeneratorData("
				"void* generator_script,const char* generator_script_fn,"
				"void* generator_supplier,const char* generator_supplier_fn"
				");"
				"]]\n"
				"package.loaded['cdef'] = { CreateGeneratorData = lib.CreateGeneratorData }";

			int load_return = luaL_loadstring(state, preload_code);
			if (load_return != LUA_OK)
			{
				using namespace std::string_literals;

				throw ENGINE_EXCEPTION("Exception loading Lua preload code! loadfile: "s
										   .append(std::to_string(load_return))
										   .append("\n\t"s)
										   .append(lua_tostring(state, -1)));
			}

			int pcall_return = lua_pcall(state, 0, LUA_MULTRET, 0);
			if (pcall_return != LUA_OK)
			{
				using namespace std::string_literals;

				throw ENGINE_EXCEPTION("Exception compiling Lua preload code! pcall: "s
										   .append(std::to_string(pcall_return))
										   .append("\n\t")
										   .append(lua_tostring(state, -1)));
			}

			return 0;
		}
	} // namespace

#pragma endregion

	void ILuaScript::PerformPreloadSteps()
	{
		lua_getglobal(state, LUA_LOADLIBNAME);
		ENGINE_EXCEPTION_ON_ASSERT(lua_istable(state, -1), "Lua Module 'package' is not loaded!")

		lua_getfield(state, -1, "preload");
		assert(lua_istable(state, -1));

		lua_pushcfunction(state, CdefLazyLoader);
		lua_setfield(state, -2, "cdef");
	}

	void ILuaScript::LoadAndCompileScript()
	{
		RegisterRequirePath(state, this);

		std::filesystem::path script_path = path;

		// Load file and compile it
		// this can raise syntax errors
		int load_return = luaL_loadfile(state, script_path.string().c_str());
		if (load_return != LUA_OK)
		{
			using namespace std::string_literals;

			throw ENGINE_EXCEPTION("Exception loading Lua script! loadfile: "s
									   .append(std::to_string(load_return))
									   .append("\n\t"s)
									   .append(lua_tostring(state, -1)));
		}

		// "Interpret" the script
		int pcall_return = lua_pcall(state, 0, LUA_MULTRET, 0);
		if (pcall_return != LUA_OK)
		{
			using namespace std::string_literals;

			throw ENGINE_EXCEPTION("Exception compiling Lua script! pcall: "s
									   .append(std::to_string(pcall_return))
									   .append("\n\t")
									   .append(lua_tostring(state, -1)));
		}

		// Uncomment this to disable JIT compiler
		// luaJIT_setmode(state, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_OFF);

		// Register c callback functions
		RegisterCallbacks(state, this->logger);

		// Register exception-handling wrapper
		RegisterWrapper(state);

		this->logger->SimpleLog<decltype(this)>(
			Logging::LogLevel::Debug,
			"Loaded and compiled Lua script: '%s'",
			script_path.string().c_str()
		);
	}

#pragma region Public functions

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
			luaJIT_profile_start(state, "fl", &ProfilerCallback, &profiler_state);
		}

		PerformPreloadSteps();
		LoadAndCompileScript();
	}

	ILuaScript::~ILuaScript()
	{
		if (enable_profiling)
		{
			luaJIT_profile_stop(state);

			this->logger->SimpleLog<decltype(this)>(
				Logging::LogLevel::Info,
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
			this->logger->SimpleLog<decltype(this)>(
				Logging::LogLevel::Error,
				"Error when calling Lua\n\tscript '%s'\n\tfunction: "
				"'%s'\n\terrorcode: %i\n%s",
				path.string().c_str(),
				function.c_str(),
				errcall,
				errormsg
			);

			return errcall;
		}

		lua_Integer ret = lua_tointeger(state, -1);

		return static_cast<int>(ret);
	}

	// NOLINTNEXTLINE(readability-make-member-function-const)
	int ScriptFunctionRef::operator()(void* data)
	{
		if (auto locked_script = script.lock())
		{
			return locked_script->CallFunction(function, data);
		}

		throw ENGINE_EXCEPTION("Could not lock script!");
	}
} // namespace Engine::Scripting
