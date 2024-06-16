#include "Rendering/Vulkan/VulkanDeviceManager.hpp"

#include "vulkan/vulkan_core.h"

#include <cstring>
#include <set>
#include <stdexcept>

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_VULKAN_DEVICE_MANAGER
#include "Logging/LoggingPrefix.hpp"

namespace Engine::Rendering
{

#pragma region Debugging callbacks

	VKAPI_ATTR static VkBool32 VKAPI_CALL VulkanDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	)
	{
		if (auto locked_logger = ((InternalEngineObject*)pUserData)->GetLogger().lock())
		{
			locked_logger->SimpleLog(
				Logging::LogLevel::Warning,
				LOGPFX_CURRENT "Vulkan validation layer reported: %s",
				pCallbackData->pMessage
			);
		}

		return VK_FALSE;
	}

	static VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger
	)
	{
		auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
			vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")
		);
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	static void DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator
	)
	{
		auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
			vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")
		);
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}

#pragma endregion

	void VulkanDeviceManager::Init(GLFWwindow* new_window)
	{
		this->window = new_window;

		this->InitVulkanInstance();
		this->CreateVulkanSurface();
		this->InitVulkanDevice();
		this->CreateVulkanLogicalDevice();
	}

	void VulkanDeviceManager::Cleanup()
	{
		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Cleaning up");

		vkDestroySurfaceKHR(this->vk_instance, this->vk_surface, nullptr);
		vkDestroyDevice(this->vk_logical_device, nullptr);
		if (this->enable_vk_validation_layers)
		{
			DestroyDebugUtilsMessengerEXT(this->vk_instance, this->vk_debug_messenger, nullptr);
		}
		vkDestroyInstance(this->vk_instance, nullptr);
	}

#pragma region Internal init functions

	void VulkanDeviceManager::InitVulkanInstance()
	{
		// Application information
		VkApplicationInfo app_info{};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = "An unknown CMEP application"; // TODO: this->windowTitle.c_str();
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pEngineName = "CMEP EngineCore";
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion = VK_API_VERSION_1_1;

		// Check validation layer support
		if (this->enable_vk_validation_layers && !this->CheckVulkanValidationLayers())
		{
			// TODO: Remove?
			this->logger->SimpleLog(
				Logging::LogLevel::Error, LOGPFX_CURRENT "Validation layer support requested but not allowed!"
			);

			throw std::runtime_error("Validation layers requested but unsupported!");
		}

		// Vulkan instance information
		VkInstanceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pApplicationInfo = &app_info;

		// Get extensions required by GLFW
		uint32_t glfw_extension_count = 0;
		const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

		// Get our required extensions
		std::vector<const char*> vk_extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

		// Enable validation layer extension if it's a debug build
		if (this->enable_vk_validation_layers)
		{
			vk_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		// Add the required extensions
		create_info.enabledExtensionCount = static_cast<uint32_t>(vk_extensions.size());
		create_info.ppEnabledExtensionNames = vk_extensions.data();

		// Enable validation layers if it's a debug build
		if (this->enable_vk_validation_layers)
		{
			create_info.enabledLayerCount = static_cast<uint32_t>(this->vk_validation_layers.size());
			create_info.ppEnabledLayerNames = this->vk_validation_layers.data();
		}
		else
		{
			// Or else we don't enable any layers
			create_info.enabledLayerCount = 0;
		}

		// Create an instance
		if (vkCreateInstance(&create_info, nullptr, &(this->vk_instance)) != VK_SUCCESS)
		{
			// TODO: Remove?
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Could not create Vulkan instance");
			throw std::runtime_error("Could not create Vulkan instance");
		}
		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Created a Vulkan instance");

		// If it's a debug build, add a debug callback to Vulkan
		if (this->enable_vk_validation_layers)
		{
			this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Creating debug messenger");
			VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info{};
			debug_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debug_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
														  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
														  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debug_messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
													  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
													  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debug_messenger_create_info.pfnUserCallback = VulkanDebugCallback;
			debug_messenger_create_info.pUserData = this;

			if (CreateDebugUtilsMessengerEXT(
					this->vk_instance, &debug_messenger_create_info, nullptr, &(this->vk_debug_messenger)
				) != VK_SUCCESS)
			{
				// TODO: Remove?
				this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Could not create a debug messenger");
				throw std::runtime_error("Could not create debug messenger!");
			}
			this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Created debug messenger");
		}
	}

	void VulkanDeviceManager::InitVulkanDevice()
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Initializing vulkan device");
		// Get physical device count
		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(this->vk_instance, &device_count, nullptr);

		// Check if there are any Vulkan-supporting devices
		if (device_count == 0)
		{
			// TODO: Remove?
			this->logger->SimpleLog(
				Logging::LogLevel::Error, LOGPFX_CURRENT "Found no device supporting the Vulkan API"
			);

			throw std::runtime_error("Found no device supporting the Vulkan API");
		}

		// Get all Vulkan-supporting devices
		std::vector<VkPhysicalDevice> physical_devices(device_count);
		vkEnumeratePhysicalDevices(this->vk_instance, &device_count, physical_devices.data());

		std::multimap<int, VkPhysicalDevice> candidates;

		for (const auto& device : physical_devices)
		{
			int score = this->CheckVulkanPhysicalDeviceScore(device);
			candidates.insert(std::make_pair(score, device));
		}

		// Check if the best candidate is suitable at all
		if (candidates.rbegin()->first > 0)
		{
			this->vk_physical_device = candidates.rbegin()->second;
			this->msaa_samples = this->GetMaxUsableSampleCount();
			this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Using MSAAx%u", this->msaa_samples);
		}

		if (this->vk_physical_device == VK_NULL_HANDLE)
		{
			// TODO: Remove?
			this->logger->SimpleLog(
				Logging::LogLevel::Error, LOGPFX_CURRENT "No suitable physical device found, fatal error"
			);
			throw std::runtime_error("No physical device found!");
		}

		VkPhysicalDeviceProperties device_properties;
		vkGetPhysicalDeviceProperties(this->vk_physical_device, &device_properties);
		this->logger->SimpleLog(
			Logging::LogLevel::Info,
			LOGPFX_CURRENT "Found a capable physical device: '%s'",
			device_properties.deviceName
		);
	}

	bool VulkanDeviceManager::CheckVulkanValidationLayers()
	{
		// Get supported validation layer count
		uint32_t layer_count;
		vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

		// Get all validation layers supported
		std::vector<VkLayerProperties> available_layers(layer_count);
		vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

		// Check if any of the supported validation layers feature the ones we want to enable
		for (const char* layer_name : this->vk_validation_layers)
		{
			bool layer_found = false;

			for (const auto& layer_properties : available_layers)
			{
				if (strcmp(layer_name, layer_properties.layerName) == 0)
				{
					layer_found = true;
					break;
				}
			}

			// If none of those we want are supported, return false
			if (!layer_found)
			{
				return false;
			}
		}

		return true;
	}

	void VulkanDeviceManager::CreateVulkanSurface()
	{
		if (glfwCreateWindowSurface(this->vk_instance, this->window, nullptr, &this->vk_surface) != VK_SUCCESS)
		{
			// TODO: Remove?
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan surface creation failed");
			throw std::runtime_error("failed to create window surface!");
		}
		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Created glfw window surface");
	}

	void VulkanDeviceManager::CreateVulkanLogicalDevice()
	{
		this->graphics_queue_indices = this->FindVulkanQueueFamilies(this->vk_physical_device);

		// Vector of queue creation structs
		std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

		// Indices of which queue families we're going to use
		std::set<uint32_t> unique_queue_families = {
			this->graphics_queue_indices.graphics_family.value(), this->graphics_queue_indices.present_family.value()
		};

		// Fill queueCreateInfos
		float queue_priority = 1.0f;
		for (uint32_t queue_family : unique_queue_families)
		{
			VkDeviceQueueCreateInfo queue_create_info{};
			queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info.queueFamilyIndex = queue_family;
			queue_create_info.queueCount = 1;
			queue_create_info.pQueuePriorities = &queue_priority;
			queue_create_infos.push_back(queue_create_info);
		}

		VkPhysicalDeviceDescriptorIndexingFeatures device_descriptor_indexing_features{};
		device_descriptor_indexing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
		device_descriptor_indexing_features.descriptorBindingPartiallyBound = VK_TRUE;

		VkPhysicalDeviceRobustness2FeaturesEXT device_robustness_features{};
		device_robustness_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
		device_robustness_features.nullDescriptor = VK_TRUE;
		device_robustness_features.pNext = &device_descriptor_indexing_features;

		VkPhysicalDeviceFeatures2 device_features2{};
		device_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		vkGetPhysicalDeviceFeatures2(this->vk_physical_device, &device_features2);
		device_features2.pNext = &device_robustness_features;

		// Logical device creation information
		VkDeviceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.pQueueCreateInfos = queue_create_infos.data();
		create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
		create_info.enabledExtensionCount = static_cast<uint32_t>(this->device_extensions.size());
		create_info.ppEnabledExtensionNames = this->device_extensions.data();
		create_info.pNext = &device_features2;

		// Set logical device extensions
		create_info.enabledExtensionCount = static_cast<uint32_t>(this->device_extensions.size());
		create_info.ppEnabledExtensionNames = this->device_extensions.data();

		// Again set validation layers, this part is apparently ignored by modern drivers
		if (this->enable_vk_validation_layers)
		{
			create_info.enabledLayerCount = static_cast<uint32_t>(this->vk_validation_layers.size());
			create_info.ppEnabledLayerNames = this->vk_validation_layers.data();
		}
		else
		{
			create_info.enabledLayerCount = 0;
		}

		// Create logical device
		VkResult result = vkCreateDevice(this->vk_physical_device, &create_info, nullptr, &this->vk_logical_device);
		if (result != VK_SUCCESS)
		{
			// TODO: Remove?
			this->logger->SimpleLog(
				Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan logical device creation failed with %u code", result
			);
			throw std::runtime_error("Vulkan: failed to create logical device!");
		}

		// Get queue handles
		vkGetDeviceQueue(
			this->vk_logical_device, this->graphics_queue_indices.graphics_family.value(), 0, &this->vk_graphics_queue
		);
		vkGetDeviceQueue(
			this->vk_logical_device, this->graphics_queue_indices.present_family.value(), 0, &this->vk_present_queue
		);
	}

#pragma endregion

#pragma region Internal device functions

	SwapChainSupportDetails VulkanDeviceManager::QueryVulkanSwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->vk_surface, &details.capabilities);

		uint32_t format_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->vk_surface, &format_count, nullptr);

		if (format_count != 0)
		{
			details.formats.resize(format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->vk_surface, &format_count, details.formats.data());
		}

		uint32_t present_mode_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->vk_surface, &present_mode_count, nullptr);

		if (present_mode_count != 0)
		{
			details.presentModes.resize(present_mode_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(
				device, this->vk_surface, &present_mode_count, details.presentModes.data()
			);
		}

		return details;
	}

	bool VulkanDeviceManager::CheckVulkanDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extension_count;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

		std::vector<VkExtensionProperties> available_extensions(extension_count);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

		std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());

		for (const auto& extension : available_extensions)
		{
			required_extensions.erase(extension.extensionName);
		}

		for (const auto& extension : required_extensions)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Warning, LOGPFX_CURRENT "Unsupported extension: %s", extension.c_str()
			);
		}

		return required_extensions.empty();
	}

	QueueFamilyIndices VulkanDeviceManager::FindVulkanQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

		std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

		uint32_t i = 0;
		for (const auto& queue_family : queue_families)
		{
			if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphics_family = i;
			}

			VkBool32 present_support = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, this->vk_surface, &present_support);

			if (present_support)
			{
				indices.present_family = i;
			}

			i++;
		}

		return indices;
	}

	int VulkanDeviceManager::CheckVulkanPhysicalDeviceScore(VkPhysicalDevice device)
	{
		// Physical device properties
		VkPhysicalDeviceProperties device_properties;
		vkGetPhysicalDeviceProperties(device, &device_properties);

		// Physical device optional features
		VkPhysicalDeviceFeatures device_features;
		vkGetPhysicalDeviceFeatures(device, &device_features);

		int score = 0;

		// Discrete GPUs have a significant performance advantage
		switch (device_properties.deviceType)
		{
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				score += 400;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				score += 300;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
				score += 200;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_CPU:
				score += 100;
				break;
			default:
				score += 5;
				break;
		}

		QueueFamilyIndices indices = this->FindVulkanQueueFamilies(device);

		// Return 0 (unsuitable) if some of the required functions aren't supported
		if (!indices.IsComplete())
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Debug2,
				LOGPFX_CURRENT "Found device '%s', unsuitable, required queue types unsupported",
				device_properties.deviceName,
				score
			);
			return 0;
		}
		else if (!this->CheckVulkanDeviceExtensionSupport(device))
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Debug2,
				LOGPFX_CURRENT "Found device '%s', unsuitable, cause: required extensions unsupported",
				device_properties.deviceName
			);
			return 0;
		}
		else if (!device_features.samplerAnisotropy)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Debug2,
				LOGPFX_CURRENT "Found device '%s', unsuitable, cause: anisotropy unsupported",
				device_properties.deviceName
			);
			return 0;
		}

		bool swap_chain_adequate = false;
		SwapChainSupportDetails swap_chain_support = this->QueryVulkanSwapChainSupport(device);
		swap_chain_adequate = !swap_chain_support.formats.empty() && !swap_chain_support.presentModes.empty();

		if (!swap_chain_adequate)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Debug2,
				LOGPFX_CURRENT "Found device '%s', unsuitable, cause: inadequate or no swap chain support",
				device_properties.deviceName
			);
			return 0;
		}

		this->logger->SimpleLog(
			Logging::LogLevel::Debug2,
			LOGPFX_CURRENT "Found device '%s', suitability %u",
			device_properties.deviceName,
			score
		);

		return score;
	}

	VkSampleCountFlagBits VulkanDeviceManager::GetMaxUsableSampleCount()
	{
		VkPhysicalDeviceProperties physical_device_properties;
		vkGetPhysicalDeviceProperties(this->vk_physical_device, &physical_device_properties);

		VkSampleCountFlags counts = physical_device_properties.limits.framebufferColorSampleCounts &
									physical_device_properties.limits.framebufferDepthSampleCounts;
		if (counts & VK_SAMPLE_COUNT_64_BIT)
		{
			return VK_SAMPLE_COUNT_64_BIT;
		}
		if (counts & VK_SAMPLE_COUNT_32_BIT)
		{
			return VK_SAMPLE_COUNT_32_BIT;
		}
		if (counts & VK_SAMPLE_COUNT_16_BIT)
		{
			return VK_SAMPLE_COUNT_16_BIT;
		}
		if (counts & VK_SAMPLE_COUNT_8_BIT)
		{
			return VK_SAMPLE_COUNT_8_BIT;
		}
		if (counts & VK_SAMPLE_COUNT_4_BIT)
		{
			return VK_SAMPLE_COUNT_4_BIT;
		}
		if (counts & VK_SAMPLE_COUNT_2_BIT)
		{
			return VK_SAMPLE_COUNT_2_BIT;
		}

		return VK_SAMPLE_COUNT_1_BIT;
	}

#pragma endregion

} // namespace Engine::Rendering
