#pragma once

#include "Scripting/ILuaScript.hpp"

#include <string>

namespace Engine::Scripting
{
	class EventLuaScript final : public ILuaScript
	{
	public:
		using ILuaScript::ILuaScript;

	private:
		void initializeCall(const std::string& function);

		int internalCall(const std::string& function, void* data) override;
	};
} // namespace Engine::Scripting
