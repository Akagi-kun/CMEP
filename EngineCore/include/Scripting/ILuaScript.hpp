#pragma once

#include "Assets/Asset.hpp"

#include "InternalEngineObject.hpp"
#include "lua.hpp"

#include <filesystem>
#include <memory>
#include <string>

namespace Engine::Scripting
{
	struct ScriptPerfState
	{
		int native_count	  = 0;
		int interpreted_count = 0;
		int engine_count	  = 0;
	};

	class ILuaScript : public InternalEngineObject, public Asset
	{
	public:
		ILuaScript(
			Engine*				  with_engine,
			std::filesystem::path with_path,
			bool				  with_enable_profiling = false
		);
		virtual ~ILuaScript();

		int callFunction(const std::string& function, void* data);

		[[nodiscard]] const ScriptPerfState& getProfilerState() const
		{
			return profiler_state;
		}

		[[nodiscard]] lua_State* getState()
		{
			return state;
		}

		[[nodiscard]] const std::filesystem::path& getPath() const
		{
			return path;
		}

	protected:
		lua_State*		state;
		ScriptPerfState profiler_state;

		std::filesystem::path path;

		void loadAndCompileScript();

		virtual int internalCall(const std::string& function, void* data) = 0;

	private:
		const bool enable_profiling;
	};

	struct ScriptFunctionRef
	{
		std::weak_ptr<ILuaScript> script;
		std::string				  function;

		int operator()(void* data);
	};
} // namespace Engine::Scripting
