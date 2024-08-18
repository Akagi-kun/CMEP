#pragma once

#include "HandleWrapper.hpp"
#include "ImportVulkan.hpp"

namespace Engine::Rendering::Vulkan
{
	class Queue : public HandleWrapper<VkQueue, true>
	{
	public:
		Queue(VkQueue with_queue) : HandleWrapper<VkQueue, true>(with_queue)
		{
		}

		void WaitQueueIdle() const
		{
			vkQueueWaitIdle(native_handle);
		}
	};
} // namespace Engine::Rendering::Vulkan
