#include "Rendering/Vulkan/VulkanDeviceManager.hpp"

#include <cstring>
#include <set>

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_VULKAN_DEVICE_MANAGER
#include "Logging/LoggingPrefix.hpp"

namespace Engine::Rendering
{

#pragma region Debugging callbacks

	VKAPI_ATTR VkBool32 VKAPI_CALL vulcanDebugCallback(
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
				LOGPFX_CURRENT "Vulcan validation layer reported: %s",
				pCallbackData->pMessage
			);
		}

		return VK_FALSE;
	}

	VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger
	)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT
		)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator
	)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT
		)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}

#pragma endregion

	VulkanDeviceManager::VulkanDeviceManager()
	{
	}

	void VulkanDeviceManager::init(GLFWwindow* new_window)
	{
		this->window = new_window;

		this->initVulkanInstance();
		this->createVulkanSurface();
		this->initVulkanDevice();
		this->createVulkanLogicalDevice();
	}

	void VulkanDeviceManager::cleanup()
	{
		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Cleaning up");

		vkDestroySurfaceKHR(this->vkInstance, this->vkSurface, nullptr);
		vkDestroyDevice(this->vkLogicalDevice, nullptr);
		if (this->enableVkValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT(this->vkInstance, this->vkDebugMessenger, nullptr);
		}
		vkDestroyInstance(this->vkInstance, nullptr);
	}

#pragma region Getters

	const VkPhysicalDevice& VulkanDeviceManager::GetPhysicalDevice() const noexcept
	{
		return this->vkPhysicalDevice;
	}

	const VkDevice& VulkanDeviceManager::GetLogicalDevice() const noexcept
	{
		return this->vkLogicalDevice;
	}

	const VkInstance& VulkanDeviceManager::GetInstance() const noexcept
	{
		return this->vkInstance;
	}

	const QueueFamilyIndices& VulkanDeviceManager::GetQueueFamilies() const noexcept
	{
		return this->graphicsQueueIndices;
	}

	const VkSurfaceKHR& VulkanDeviceManager::GetSurface() const noexcept
	{
		return this->vkSurface;
	}

	const VkSampleCountFlagBits& VulkanDeviceManager::GetMSAASampleCount() const noexcept
	{
		return this->msaaSamples;
	}

	const VkQueue& VulkanDeviceManager::GetGraphicsQueue() const noexcept
	{
		return this->vkGraphicsQueue;
	}

	const VkQueue& VulkanDeviceManager::GetPresentQueue() const noexcept
	{
		return this->vkPresentQueue;
	}

#pragma endregion

#pragma region Trampoline functions

	SwapChainSupportDetails VulkanDeviceManager::QuerySwapChainSupport()
	{
		return this->queryVulkanSwapChainSupport(this->vkPhysicalDevice);
	}

#pragma endregion

#pragma region Internal init functions

	void VulkanDeviceManager::initVulkanInstance()
	{
		// Application information
		VkApplicationInfo app_info{};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = "NIL_APP"; // TODO: this->windowTitle.c_str();
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pEngineName = "CMEP";
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion = VK_API_VERSION_1_1;

		// Check validation layer support
		if (this->enableVkValidationLayers && !this->checkVulkanValidationLayers())
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Error, LOGPFX_CURRENT "Validation layer support requested but not allowed!"
			);
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
		if (this->enableVkValidationLayers)
		{
			vk_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		// Add the required extensions
		create_info.enabledExtensionCount = static_cast<uint32_t>(vk_extensions.size());
		create_info.ppEnabledExtensionNames = vk_extensions.data();

		// Enable validation layers if it's a debug build
		if (this->enableVkValidationLayers)
		{
			create_info.enabledLayerCount = static_cast<uint32_t>(this->vkValidationLayers.size());
			create_info.ppEnabledLayerNames = this->vkValidationLayers.data();
		}
		else
		{
			// Or else we don't enable any layers
			create_info.enabledLayerCount = 0;
		}

		// Create an instance
		if (vkCreateInstance(&create_info, nullptr, &(this->vkInstance)) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Exception, LOGPFX_CURRENT "Could not create Vulkan instance");
			throw std::runtime_error("Could not create Vulkan instance");
		}
		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Created a Vulkan instance");

		// If it's a debug build, add a debug callback to Vulkan
		if (this->enableVkValidationLayers)
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
			debug_messenger_create_info.pfnUserCallback = vulcanDebugCallback;
			debug_messenger_create_info.pUserData = this;

			if (CreateDebugUtilsMessengerEXT(
					this->vkInstance, &debug_messenger_create_info, nullptr, &(this->vkDebugMessenger)
				) != VK_SUCCESS)
			{
				this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Could not create a debug messenger");
			}
			this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Created debug messenger");
		}
	}

	void VulkanDeviceManager::initVulkanDevice()
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Initializing vulkan device");
		// Get physical device count
		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(this->vkInstance, &device_count, nullptr);

		// Check if there are any Vulkan-supporting devices
		if (device_count == 0)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Error, LOGPFX_CURRENT "Found no device supporting the Vulcan API"
			);
		}

		// Get all Vulkan-supporting devices
		std::vector<VkPhysicalDevice> physical_devices(device_count);
		vkEnumeratePhysicalDevices(this->vkInstance, &device_count, physical_devices.data());

		std::multimap<int, VkPhysicalDevice> candidates;

		for (const auto& device : physical_devices)
		{
			int score = this->checkVulkanPhysicalDeviceScore(device);
			candidates.insert(std::make_pair(score, device));
		}

		// Check if the best candidate is suitable at all
		if (candidates.rbegin()->first > 0)
		{
			this->vkPhysicalDevice = candidates.rbegin()->second;
			this->msaaSamples = this->getMaxUsableSampleCount();
			this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Using MSAAx%u", this->msaaSamples);
		}

		if (this->vkPhysicalDevice == VK_NULL_HANDLE)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Exception, LOGPFX_CURRENT "No suitable physical device found, fatal error"
			);
			throw std::runtime_error("FATAL! No physical device found");
		}

		VkPhysicalDeviceProperties device_properties;
		vkGetPhysicalDeviceProperties(this->vkPhysicalDevice, &device_properties);
		this->logger->SimpleLog(
			Logging::LogLevel::Info,
			LOGPFX_CURRENT "Found a capable physical device: '%s'",
			device_properties.deviceName
		);
	}

	bool VulkanDeviceManager::checkVulkanValidationLayers()
	{
		// Get supported validation layer count
		uint32_t layer_count;
		vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

		// Get all validation layers supported
		std::vector<VkLayerProperties> available_layers(layer_count);
		vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

		// Check if any of the supported validation layers feature the ones we want to enable
		for (const char* layer_name : this->vkValidationLayers)
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

	void VulkanDeviceManager::createVulkanSurface()
	{
		if (glfwCreateWindowSurface(this->vkInstance, this->window, nullptr, &this->vkSurface) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Exception, LOGPFX_CURRENT "Vulkan surface creation failed");
			throw std::runtime_error("failed to create window surface!");
		}
		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Created glfw window surface");
	}

	void VulkanDeviceManager::createVulkanLogicalDevice()
	{
		this->graphicsQueueIndices = this->findVulkanQueueFamilies(this->vkPhysicalDevice);

		// Vector of queue creation structs
		std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

		// Indices of which queue families we're going to use
		std::set<uint32_t> unique_queue_families = {
			this->graphicsQueueIndices.graphics_family.value(), this->graphicsQueueIndices.present_family.value()
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
		vkGetPhysicalDeviceFeatures2(this->vkPhysicalDevice, &device_features2);
		device_features2.pNext = &device_robustness_features;

		// Logical device creation information
		VkDeviceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.pQueueCreateInfos = queue_create_infos.data();
		create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
		create_info.enabledExtensionCount = static_cast<uint32_t>(this->deviceExtensions.size());
		create_info.ppEnabledExtensionNames = this->deviceExtensions.data();
		create_info.pNext = &device_features2;

		// Set logical device extensions
		create_info.enabledExtensionCount = static_cast<uint32_t>(this->deviceExtensions.size());
		create_info.ppEnabledExtensionNames = this->deviceExtensions.data();

		// Again set validation layers, this part is apparently ignored by modern drivers
		if (this->enableVkValidationLayers)
		{
			create_info.enabledLayerCount = static_cast<uint32_t>(this->vkValidationLayers.size());
			create_info.ppEnabledLayerNames = this->vkValidationLayers.data();
		}
		else
		{
			create_info.enabledLayerCount = 0;
		}

		// Create logical device
		VkResult result = vkCreateDevice(this->vkPhysicalDevice, &create_info, nullptr, &this->vkLogicalDevice);
		if (result != VK_SUCCESS)
		{
			this->logger->SimpleLog(
				Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan logical device creation failed with %u code", result
			);
			throw std::runtime_error("Vulkan: failed to create logical device!");
		}

		// Get queue handles
		vkGetDeviceQueue(
			this->vkLogicalDevice, this->graphicsQueueIndices.graphics_family.value(), 0, &this->vkGraphicsQueue
		);
		vkGetDeviceQueue(
			this->vkLogicalDevice, this->graphicsQueueIndices.present_family.value(), 0, &this->vkPresentQueue
		);
	}

#pragma endregion

#pragma region Internal device functions

	SwapChainSupportDetails VulkanDeviceManager::queryVulkanSwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->vkSurface, &details.capabilities);

		uint32_t format_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->vkSurface, &format_count, nullptr);

		if (format_count != 0)
		{
			details.formats.resize(format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->vkSurface, &format_count, details.formats.data());
		}

		uint32_t present_mode_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->vkSurface, &present_mode_count, nullptr);

		if (present_mode_count != 0)
		{
			details.presentModes.resize(present_mode_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(
				device, this->vkSurface, &present_mode_count, details.presentModes.data()
			);
		}

		return details;
	}

	bool VulkanDeviceManager::checkVulkanDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extension_count;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

		std::vector<VkExtensionProperties> available_extensions(extension_count);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

		std::set<std::string> required_extensions(deviceExtensions.begin(), deviceExtensions.end());

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

	QueueFamilyIndices VulkanDeviceManager::findVulkanQueueFamilies(VkPhysicalDevice device)
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
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, this->vkSurface, &present_support);

			if (present_support)
			{
				indices.present_family = i;
			}

			i++;
		}

		return indices;
	}

	int VulkanDeviceManager::checkVulkanPhysicalDeviceScore(VkPhysicalDevice device)
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
				score += 1000;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				score += 800;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
				score += 600;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_CPU:
				score += 200;
				break;
			default:
				score += 5;
				break;
		}

		QueueFamilyIndices indices = this->findVulkanQueueFamilies(device);

		// Return 0 (unsuitable) if some of the required queues aren't supported
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
		else if (!this->checkVulkanDeviceExtensionSupport(device))
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
		SwapChainSupportDetails swap_chain_support = this->queryVulkanSwapChainSupport(device);
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

	VkSampleCountFlagBits VulkanDeviceManager::getMaxUsableSampleCount()
	{
		VkPhysicalDeviceProperties physical_device_properties;
		vkGetPhysicalDeviceProperties(this->vkPhysicalDevice, &physical_device_properties);

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
