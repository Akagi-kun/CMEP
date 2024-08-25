#pragma once

#include "fwd.hpp"

namespace Engine::Rendering::Vulkan
{
	class InstanceOwned
	{
	public:
		using value_t = Instance*;

		InstanceOwned() = default;
		InstanceOwned(value_t with_instance) : instance(with_instance)
		{
		}

	protected:
		value_t instance = nullptr;
	};
} // namespace Engine::Rendering::Vulkan