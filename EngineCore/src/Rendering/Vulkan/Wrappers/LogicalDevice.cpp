#include "Rendering/Vulkan/Wrappers/LogicalDevice.hpp"

#include "Rendering/Vulkan/Wrappers/DeviceScore.hpp"
#include "Rendering/Vulkan/Wrappers/Instance.hpp"
#include "Rendering/Vulkan/Wrappers/InstanceOwned.hpp"

#include "vulkan/vulkan_core.h"

#include <set>
#include <stdexcept>

#ifdef DS_VALIDATION_LAYERS_ENABLED
#	pragma message("Validation layers enabled")
#endif

namespace Engine::Rendering::Vulkan
{
	const std::vector<const char*> LogicalDevice::vk_validation_layers = {
		"VK_LAYER_KHRONOS_validation",
	};

#pragma region Public

	LogicalDevice::LogicalDevice(InstanceOwned::value_t with_instance, const Surface* with_surface)
		: InstanceOwned(with_instance)
	{
		auto physical_device = with_instance->GetPhysicalDevice();

		queue_family_indices = physical_device.FindVulkanQueueFamilies(with_surface);

		// Vector of queue creation structs
		std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

		// Indices of which queue families we're going to use
		std::set<uint32_t> unique_queue_families = {
			queue_family_indices.graphics_family.value(),
			queue_family_indices.present_family.value()
		};

		// Fill queueCreateInfos
		float queue_priority = 1.0f;
		for (uint32_t queue_family : unique_queue_families)
		{
			VkDeviceQueueCreateInfo queue_create_info{};
			queue_create_info.sType			   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info.queueFamilyIndex = queue_family;
			queue_create_info.queueCount	   = 1;
			queue_create_info.pQueuePriorities = &queue_priority;
			queue_create_infos.push_back(queue_create_info);
		}

		VkPhysicalDeviceDescriptorIndexingFeatures device_descriptor_indexing_features{};
		device_descriptor_indexing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
		device_descriptor_indexing_features.descriptorBindingPartiallyBound = VK_TRUE;

		VkPhysicalDeviceRobustness2FeaturesEXT device_robustness_features{};
		device_robustness_features.sType		  = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
		device_robustness_features.nullDescriptor = VK_TRUE;
		device_robustness_features.pNext		  = &device_descriptor_indexing_features;

		VkPhysicalDeviceFeatures2 device_features2{};
		device_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		vkGetPhysicalDeviceFeatures2(physical_device, &device_features2);
		device_features2.pNext = &device_robustness_features;

		// Logical device creation information
		VkDeviceCreateInfo create_info{};
		create_info.sType				 = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.pQueueCreateInfos	 = queue_create_infos.data();
		create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
		create_info.pNext				 = &device_features2;

		// Set logical device extensions
		create_info.enabledExtensionCount	= static_cast<uint32_t>(DeviceScore::device_extensions.size());
		create_info.ppEnabledExtensionNames = DeviceScore::device_extensions.data();

		// Again set validation layers, this part is apparently ignored by modern drivers
		if (enable_vk_validation_layers)
		{
			create_info.enabledLayerCount	= static_cast<uint32_t>(vk_validation_layers.size());
			create_info.ppEnabledLayerNames = vk_validation_layers.data();
		}
		else
		{
			create_info.enabledLayerCount = 0;
		}

		// Create logical device
		VkResult result = vkCreateDevice(physical_device, &create_info, nullptr, &native_handle);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Vulkan: failed to create logical device!");
		}

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
		vkDestroyDevice(native_handle, nullptr);
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
