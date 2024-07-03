#pragma once

#include "Rendering/Vulkan/ImportVulkan.hpp"

namespace Engine::Rendering
{
	class HoldsVMA
	{
	protected:
		VmaAllocator allocator = nullptr;

	public:
		HoldsVMA() = delete;
		HoldsVMA(VmaAllocator with_allocator) : allocator(with_allocator)
		{
		}
	};
} // namespace Engine::Rendering
