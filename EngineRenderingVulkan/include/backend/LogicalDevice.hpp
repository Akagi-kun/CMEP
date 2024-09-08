#pragma once
// IWYU pragma: private; include Rendering/Vulkan/backend.hpp

#include "fwd.hpp"

#include "common/InstanceOwned.hpp"
#include "common/StructDefs.hpp"
#include "vulkan/vulkan_raii.hpp"

namespace Engine::Rendering::Vulkan
{
	class LogicalDevice : public InstanceOwned, public vk::raii::Device
	{
	public:
		LogicalDevice(LogicalDevice&) = delete;

		LogicalDevice(InstanceOwned::value_t with_instance, const Surface* with_surface);

		[[nodiscard]] const QueueFamilyIndices& getQueueFamilies() const
		{
			return queue_family_indices;
		}

		[[nodiscard]] vk::raii::Queue& getGraphicsQueue()
		{
			return graphics_queue;
		}

		[[nodiscard]] vk::raii::Queue& getPresentQueue()
		{
			return present_queue;
		}

	private:
		QueueFamilyIndices queue_family_indices{};
		vk::raii::Queue	   graphics_queue = nullptr;
		vk::raii::Queue	   present_queue  = nullptr;

		vk::raii::Device createDevice(Instance* with_instance, const Surface* with_surface);
	};
} // namespace Engine::Rendering::Vulkan
