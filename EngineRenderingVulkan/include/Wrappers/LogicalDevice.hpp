#pragma once

#include "HandleWrapper.hpp"
#include "VulkanStructDefs.hpp"
#include "Wrappers/InstanceOwned.hpp"
#include "Wrappers/Queue.hpp"

// #include "hpp"
#include "ImportVulkan.hpp"

namespace Engine::Rendering::Vulkan
{
	class LogicalDevice : public InstanceOwned, public HandleWrapper<vk::Device>
	{
	public:
		LogicalDevice(LogicalDevice& other) = delete;

		LogicalDevice(InstanceOwned::value_t with_instance, const Surface* with_surface);
		~LogicalDevice();

		[[nodiscard]] const QueueFamilyIndices& GetQueueFamilies() const
		{
			return queue_family_indices;
		}

		[[nodiscard]] Queue& GetGraphicsQueue()
		{
			return graphics_queue;
		}

		[[nodiscard]] Queue& GetPresentQueue()
		{
			return present_queue;
		}

	private:
		QueueFamilyIndices queue_family_indices{};
		Queue graphics_queue;
		Queue present_queue;

		static const std::vector<const char*> vk_validation_layers;

		[[nodiscard]] Queue GetDeviceQueue(uint32_t family, uint32_t index) const;
	};
} // namespace Engine::Rendering::Vulkan
