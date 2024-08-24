#pragma once

#include "Scripting/ILuaScript.hpp"

namespace Engine::Scripting
{
	class EventLuaScript final : public ILuaScript
	{
	public:
		using ILuaScript::ILuaScript;

	private:
		void InitializeCall(const std::string& function);

		int InternalCall(const std::string& function, void* data) override;
	};
} // namespace Engine::Scripting
