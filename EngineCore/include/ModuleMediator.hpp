#pragma once

#include "IModule.hpp"

namespace Engine
{
	class ModuleMediator
	{
	public:
		virtual ~ModuleMediator() = default;

		virtual void ModuleBroadcast(ModuleType for_type, const ModuleMessage& data) = 0;
	};
} // namespace Engine
