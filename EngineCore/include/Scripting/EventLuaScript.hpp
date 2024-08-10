#pragma once

#include "Scripting/ILuaScript.hpp"

namespace Engine::Scripting
{
	class EventLuaScript final : public ILuaScript
	{
	private:
		void InitializeCall(const std::string& function);

		int InternalCall(const std::string& function, void* data) override;

	public:
		using ILuaScript::ILuaScript;
	};
} // namespace Engine::Scripting
