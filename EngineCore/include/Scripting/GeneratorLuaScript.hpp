#pragma once

#include "Scripting/ILuaScript.hpp"

namespace Engine::Scripting
{
	class GeneratorLuaScript final : public ILuaScript
	{
	private:
		int InternalCall(const std::string& function, void* data) override;

	public:
		using ILuaScript::ILuaScript;
	};
} // namespace Engine::Scripting
