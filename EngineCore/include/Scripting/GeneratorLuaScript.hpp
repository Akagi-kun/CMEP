#pragma once

#include "Scripting/ILuaScript.hpp"

#include "InternalEngineObject.hpp"

#include <filesystem>
#include <string>

namespace Engine::Scripting
{
	class GeneratorLuaScript final : public ILuaScript
	{
	public:
		GeneratorLuaScript(Engine* with_engine, const std::filesystem::path& with_path);

	private:
		int internalCall(const std::string& function, void* data) override;
	};
} // namespace Engine::Scripting
