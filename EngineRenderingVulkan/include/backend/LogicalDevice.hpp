#pragma once

#include "ImportVulkan.hpp"
#include "common/InstanceOwned.hpp"
#include "common/StructDefs.hpp"

namespace Engine::Rendering::Vulkan
{
	class LogicalDevice : public InstanceOwned, public vk::raii::Device
	{
	public:
		LogicalDevice(LogicalDevice&) = delete;

		LogicalDevice(InstanceOwned::value_t with_instance, const Surface* with_surface);

		[[nodiscard]] const QueueFamilyIndices& GetQueueFamilies() const
		{
			return queue_family_indices;
		}

		[[nodiscard]] vk::raii::Queue& GetGraphicsQueue()
		{
			return graphics_queue;
		}

		[[nodiscard]] vk::raii::Queue& GetPresentQueue()
		{
			return present_queue;
		}

	private:
		QueueFamilyIndices queue_family_indices{};
		vk::raii::Queue graphics_queue = nullptr;
		vk::raii::Queue present_queue  = nullptr;

		vk::raii::Device CreateDevice(Instance* with_instance, const Surface* with_surface);
		// static const std::vector<const char*> device_validation_layers;
	};
} // namespace Engine::Rendering::Vulkan
