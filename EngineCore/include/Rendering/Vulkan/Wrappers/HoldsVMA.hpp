#pragma once

#include "MemoryAllocator.hpp"

namespace Engine::Rendering::Vulkan
{
	class HoldsVMA
	{
	protected:
		MemoryAllocator* allocator = nullptr;

	public:
		HoldsVMA() = delete;
		HoldsVMA(MemoryAllocator* with_allocator) : allocator(with_allocator)
		{
		}
	};
} // namespace Engine::Rendering::Vulkan
