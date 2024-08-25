#pragma once

#include "Scripting/ILuaScript.hpp"

namespace Engine::Scripting
{
	class GeneratorLuaScript final : public ILuaScript
	{
	public:
		GeneratorLuaScript(Engine* with_engine, const std::filesystem::path& with_path);

	private:
		int InternalCall(const std::string& function, void* data) override;
	};
} // namespace Engine::Scripting
