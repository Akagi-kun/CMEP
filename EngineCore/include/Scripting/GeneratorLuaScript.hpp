#pragma once

#include "Scripting/ILuaScript.hpp"

namespace Engine::Scripting
{
	class GeneratorLuaScript final : public ILuaScript
	{
	public:
		GeneratorLuaScript(Engine* with_engine, std::string with_path);

	private:
		int InternalCall(const std::string& function, void* data) override;
	};
} // namespace Engine::Scripting
