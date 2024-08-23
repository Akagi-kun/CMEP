#include "backend/LogicalDevice.hpp"

#include "ImportVulkan.hpp"
#include "backend/DeviceScore.hpp"
#include "backend/Instance.hpp"
#include "common/InstanceOwned.hpp"

#include <set>
#include <stdexcept>

namespace Engine::Rendering::Vulkan
{
#ifdef DS_VALIDATION_LAYERS_ENABLED
	const std::vector<const char*> device_validation_layers = {
		"VK_LAYER_KHRONOS_validation",
	};
#	pragma message("Validation layers enabled")
#else
	const std::vector<const char*> device_validation_layers = {};
#	pragma message("Validation layers disabled")
#endif

#pragma region Public

	vk::raii::Device LogicalDevice::CreateDevice(Instance* with_instance, const Surface* with_surface)
	{
		auto* physical_device = with_instance->GetPhysicalDevice();

		auto families = physical_device->FindVulkanQueueFamilies(with_surface);

		if (!families.has_value())
		{
			throw std::runtime_error("Not all required queue families were present!");
		}

		queue_family_indices = families.value();

		// Vector of queue creation structs
		std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;

		// Indices of which queue families we're going to use
		std::set<uint32_t> unique_queue_families = {
			queue_family_indices.graphics_family,
			queue_family_indices.present_family
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

		vk::PhysicalDeviceFeatures2 device_features2 = physical_device->getFeatures2().setPNext(
			&device_robustness_features
		);

		// Logical device creation information
		vk::DeviceCreateInfo create_info(
			{},
			queue_create_infos,
			device_validation_layers,
			DeviceScore::device_extensions,
			{},
			&device_features2
		);

		// Create logical device
		return physical_device->createDevice(create_info);
	}

	LogicalDevice::LogicalDevice(InstanceOwned::value_t with_instance, const Surface* with_surface)
		: InstanceOwned(with_instance), vk::raii::Device(CreateDevice(with_instance, with_surface))
	{ /*
		 auto* physical_device = with_instance->GetPhysicalDevice();

		 auto families = physical_device->FindVulkanQueueFamilies(with_surface);

		 if (!families.has_value())
		 {
			 throw std::runtime_error("Not all required queue families were present!");
		 }

		 queue_family_indices = families.value();

		 // Vector of queue creation structs
		 std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;

		 // Indices of which queue families we're going to use
		 std::set<uint32_t> unique_queue_families = {
			 queue_family_indices.graphics_family,
			 queue_family_indices.present_family
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

		 vk::PhysicalDeviceFeatures2 device_features2 = physical_device->getFeatures2().setPNext(
			 &device_robustness_features
		 );

		 // Logical device creation information
		 vk::DeviceCreateInfo create_info(
			 {},
			 queue_create_infos,
			 device_validation_layers,
			 DeviceScore::device_extensions,
			 {},
			 &device_features2
		 );

		 // Create logical device
		 native_handle = physical_device->createDevice(create_info);
  */
		// Get queue handles
		graphics_queue = getQueue(queue_family_indices.graphics_family, 0);
		present_queue  = getQueue(queue_family_indices.present_family, 0);
	}

#pragma endregion

} // namespace Engine::Rendering::Vulkan
