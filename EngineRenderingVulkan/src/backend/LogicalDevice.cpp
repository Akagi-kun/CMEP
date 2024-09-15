#include "backend/LogicalDevice.hpp"

#include "backend/DeviceScore.hpp"
#include "backend/Instance.hpp"
#include "common/InstanceOwned.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <cstdint>
#include <set>
#include <stdexcept>
#include <vector>

namespace Engine::Rendering::Vulkan
{
#ifdef DS_VALIDATION_LAYERS_ENABLED
	const std::vector<const char*> device_validation_layers = {
		"VK_LAYER_KHRONOS_validation",
	};
#	pragma message("-- Validation layers enabled")
#else
	const std::vector<const char*> device_validation_layers = {};
#	pragma message("-- Validation layers disabled")
#endif

#pragma region Public

	LogicalDevice::LogicalDevice(
		InstanceOwned::value_t with_instance,
		const Surface*		   with_surface
	)
		: InstanceOwned(with_instance),
		  vk::raii::Device(createDevice(with_instance, with_surface))
	{
		// Get queue handles
		graphics_queue = getQueue(queue_family_indices.graphics_family, 0);
		present_queue  = getQueue(queue_family_indices.present_family, 0);
	}

#pragma endregion

#pragma region Private

	vk::raii::Device LogicalDevice::createDevice(
		Instance*	   with_instance,
		const Surface* with_surface
	)
	{
		auto* physical_device = with_instance->getPhysicalDevice();

		auto families = physical_device->findVulkanQueueFamilies(with_surface);

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
		float queue_priorities[] = {1.0f};
		for (uint32_t queue_family : unique_queue_families)
		{
			vk::DeviceQueueCreateInfo queue_create_info{
				.queueFamilyIndex = queue_family,
				.queueCount		  = 1,
				.pQueuePriorities = queue_priorities
			};

			queue_create_infos.push_back(queue_create_info);
		}

		// Fill structs for required features and extensions
		vk::PhysicalDeviceDescriptorIndexingFeatures device_descriptor_indexing_features{
			.descriptorBindingPartiallyBound = vk::True
		};

		vk::PhysicalDeviceRobustness2FeaturesEXT device_robustness_features{
			.pNext				 = &device_descriptor_indexing_features,
			.robustBufferAccess2 = vk::True,
		};

		vk::PhysicalDeviceFeatures2 device_features2 =
			physical_device->getFeatures2().setPNext(&device_robustness_features);

		// Logical device creation information
		vk::DeviceCreateInfo create_info{
			.pNext				  = &device_features2,
			.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size()),
			.pQueueCreateInfos	  = queue_create_infos.data(),
			.enabledLayerCount	 = static_cast<uint32_t>(device_validation_layers.size()),
			.ppEnabledLayerNames = device_validation_layers.data(),
			.enabledExtensionCount =
				static_cast<uint32_t>(DeviceScore::device_extensions.size()),
			.ppEnabledExtensionNames = DeviceScore::device_extensions.data(),
			.pEnabledFeatures		 = nullptr,
		};

		// Create logical device
		return physical_device->createDevice(create_info);
	}

#pragma endregion

} // namespace Engine::Rendering::Vulkan
