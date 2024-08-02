#pragma once

#include "Scripting/ILuaScript.hpp"

namespace Engine::Scripting
{
	class GeneratorLuaScript final : public ILuaScript
	{
	private:
		int InternalCall(const std::string& function, void* data) override;

	public:
		GeneratorLuaScript(Engine* with_engine, std::string with_path);
	};
} // namespace Engine::Scripting
