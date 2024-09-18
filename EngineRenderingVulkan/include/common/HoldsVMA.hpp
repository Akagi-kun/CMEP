#pragma once

#include "fwd.hpp"

namespace Engine::Rendering::Vulkan
{
	class HoldsVMA
	{
	public:
		HoldsVMA() = default;
		HoldsVMA(MemoryAllocator* with_allocator) : allocator(with_allocator)
		{}

	protected:
		MemoryAllocator* allocator = nullptr;
	};
} // namespace Engine::Rendering::Vulkan
