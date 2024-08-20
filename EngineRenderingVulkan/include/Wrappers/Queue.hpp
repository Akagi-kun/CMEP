#pragma once

#include "HandleWrapper.hpp"
#include "ImportVulkan.hpp"

namespace Engine::Rendering::Vulkan
{
	class Queue : public HandleWrapper<vk::Queue, true>
	{
	public:
		Queue() = default;
		Queue(vk::Queue with_queue) : HandleWrapper<vk::Queue, true>(with_queue)
		{
		}

		void WaitQueueIdle() const
		{
			native_handle.waitIdle();
		}
	};
} // namespace Engine::Rendering::Vulkan
