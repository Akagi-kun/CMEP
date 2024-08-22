#pragma once

#include "ImportVulkan.hpp"
#include "common/HandleWrapper.hpp"
#include "common/InstanceOwned.hpp"
#include "common/StructDefs.hpp"

namespace Engine::Rendering::Vulkan
{
	class LogicalDevice : public InstanceOwned, public HandleWrapper<vk::raii::Device>
	{
	public:
		LogicalDevice(LogicalDevice& other) = delete;

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

		static const std::vector<const char*> vk_validation_layers;
	};
} // namespace Engine::Rendering::Vulkan
