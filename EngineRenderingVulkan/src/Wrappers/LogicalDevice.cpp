#include "Wrappers/LogicalDevice.hpp"

#include "ImportVulkan.hpp"
#include "Wrappers/DeviceScore.hpp"
#include "Wrappers/Instance.hpp"
#include "Wrappers/InstanceOwned.hpp"

#include <set>
#include <stdexcept>

namespace Engine::Rendering::Vulkan
{
#ifdef DS_VALIDATION_LAYERS_ENABLED
	const std::vector<const char*> LogicalDevice::vk_validation_layers = {
		"VK_LAYER_KHRONOS_validation",
	};
#	pragma message("Validation layers enabled")
#else
	const std::vector<const char*> LogicalDevice::vk_validation_layers = {};
#	pragma message("Validation layers disabled")
#endif

#pragma region Public

	LogicalDevice::LogicalDevice(InstanceOwned::value_t with_instance, const Surface* with_surface)
		: InstanceOwned(with_instance)
	{
		auto physical_device = with_instance->GetPhysicalDevice();

		queue_family_indices = physical_device.FindVulkanQueueFamilies(with_surface);

		// Vector of queue creation structs
		std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;

		// Indices of which queue families we're going to use
		std::set<uint32_t> unique_queue_families = {
			queue_family_indices.graphics_family.value(),
			queue_family_indices.present_family.value()
		};

		// Fill queueCreateInfos
		float queue_priority = 1.0f;
		for (uint32_t queue_family : unique_queue_families)
		{
			vk::DeviceQueueCreateInfo queue_create_info({}, queue_family, 1, &queue_priority);

			queue_create_infos.push_back(queue_create_info);
		}

		vk::PhysicalDeviceDescriptorIndexingFeatures device_descriptor_indexing_features({});
		device_descriptor_indexing_features.setDescriptorBindingPartiallyBound(vk::True);

		vk::PhysicalDeviceRobustness2FeaturesEXT
			device_robustness_features({}, {}, vk::True, &device_descriptor_indexing_features);

		vk::PhysicalDeviceFeatures2 device_features2 = physical_device.GetHandle().getFeatures2().setPNext(
			&device_robustness_features
		);

		// Logical device creation information
		vk::DeviceCreateInfo create_info(
			{},
			queue_create_infos,
			vk_validation_layers,
			DeviceScore::device_extensions,
			{},
			&device_features2
		);

		// Create logical device
		native_handle = physical_device.GetHandle().createDevice(create_info);

		if (!queue_family_indices.IsComplete())
		{
			throw std::runtime_error("Graphics indices are not complete!");
		}

		// Get queue handles
		graphics_queue = GetDeviceQueue(queue_family_indices.graphics_family.value(), 0);
		present_queue  = GetDeviceQueue(queue_family_indices.present_family.value(), 0);
	}

	LogicalDevice::~LogicalDevice()
	{
		native_handle.destroy();
	}

#pragma endregion

#pragma region Private

	[[nodiscard]] Queue LogicalDevice::GetDeviceQueue(uint32_t family, uint32_t index) const
	{
		VkQueue native;
		vkGetDeviceQueue(native_handle, family, index, &native);

		return {native};
	}

#pragma endregion

} // namespace Engine::Rendering::Vulkan
