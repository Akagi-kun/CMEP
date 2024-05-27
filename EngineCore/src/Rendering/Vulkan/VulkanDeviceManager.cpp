#include <set>

#include "Rendering/Vulkan/VulkanDeviceManager.hpp"

// Prefixes for logging messages
// #define LOGPFX_CURRENT LOGPFX_CLASS_VULKAN_RENDERING_ENGINE
#include "Logging/LoggingPrefix.hpp"

namespace Engine::Rendering
{

#pragma region Debugging callbacks

	VKAPI_ATTR VkBool32 VKAPI_CALL vulcanDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
		void *pUserData)
	{
		if (auto locked_logger = ((InternalEngineObject *)pUserData)->GetLogger().lock())
		{
			locked_logger->SimpleLog(Logging::LogLevel::Warning, LOGPFX_CURRENT "Vulcan validation layer reported: %s", pCallbackData->pMessage);
		}

		return VK_FALSE;
	}

	VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
		const VkAllocationCallbacks *pAllocator,
		VkDebugUtilsMessengerEXT *pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}

#pragma endregion

	VulkanDeviceManager::VulkanDeviceManager()
	{
	}

	void VulkanDeviceManager::init(GLFWwindow *new_window)
	{
		this->window = new_window;

		this->initVulkanInstance();
		this->createVulkanSurface();
		this->initVulkanDevice();
		this->createVulkanLogicalDevice();
	}

	void VulkanDeviceManager::cleanup()
	{
		vkDestroySurfaceKHR(this->vkInstance, this->vkSurface, nullptr);
		vkDestroyDevice(this->vkLogicalDevice, nullptr);
		if (this->enableVkValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT(this->vkInstance, this->vkDebugMessenger, nullptr);
		}
		vkDestroyInstance(this->vkInstance, nullptr);
	}

#pragma region Getters

	const VkPhysicalDevice &VulkanDeviceManager::GetPhysicalDevice() const noexcept
	{
		return this->vkPhysicalDevice;
	}

	const VkDevice &VulkanDeviceManager::GetLogicalDevice() const noexcept
	{
		return this->vkLogicalDevice;
	}

	const VkInstance &VulkanDeviceManager::GetInstance() const noexcept
	{
		return this->vkInstance;
	}

	const QueueFamilyIndices &VulkanDeviceManager::GetQueueFamilies() const noexcept
	{
		return this->graphicsQueueIndices;
	}

	const VkSurfaceKHR &VulkanDeviceManager::GetSurface() const noexcept
	{
		return this->vkSurface;
	}

	const VkSampleCountFlagBits &VulkanDeviceManager::GetMSAASampleCount() const noexcept
	{
		return this->msaaSamples;
	}

	const VkQueue &VulkanDeviceManager::GetGraphicsQueue() const noexcept
	{
		return this->vkGraphicsQueue;
	}
/* 
	const VkQueue &VulkanDeviceManager::GetPresentQueue() const noexcept
	{
		return this->vkPresentQueue;
	}
 */
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
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "NIL_APP"; // TODO: this->windowTitle.c_str();
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "CMEP";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_1;

		// Check validation layer support
		if (this->enableVkValidationLayers && !this->checkVulkanValidationLayers())
		{
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Validation layer support requested but not allowed!");
		}

		// Vulkan instance information
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// Get extensions required by GLFW
		uint32_t glfwExtensionCount = 0;
		const char **glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		// Get our required extensions
		std::vector<const char *> vkExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		// Enable validation layer extension if it's a debug build
		if (this->enableVkValidationLayers)
		{
			vkExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		// Add the required extensions
		createInfo.enabledExtensionCount = static_cast<uint32_t>(vkExtensions.size());
		createInfo.ppEnabledExtensionNames = vkExtensions.data();

		// Enable validation layers if it's a debug build
		if (this->enableVkValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(this->vkValidationLayers.size());
			createInfo.ppEnabledLayerNames = this->vkValidationLayers.data();
		}
		else
		{
			// Or else we don't enable any layers
			createInfo.enabledLayerCount = 0;
		}

		// Create an instance
		if (vkCreateInstance(&createInfo, nullptr, &(this->vkInstance)) != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Exception, LOGPFX_CURRENT "Could not create Vulkan instance");
			throw std::runtime_error("Could not create Vulkan instance");
		}
		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Created a Vulkan instance");

		// If it's a debug build, add a debug callback to Vulkan
		if (this->enableVkValidationLayers)
		{
			this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Creating debug messenger");
			VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
			debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debugMessengerCreateInfo.pfnUserCallback = vulcanDebugCallback;
			debugMessengerCreateInfo.pUserData = this;

			if (CreateDebugUtilsMessengerEXT(this->vkInstance, &debugMessengerCreateInfo, nullptr, &(this->vkDebugMessenger)) != VK_SUCCESS)
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
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(this->vkInstance, &deviceCount, nullptr);

		// Check if there are any Vulkan-supporting devices
		if (deviceCount == 0)
		{
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Found no device supporting the Vulcan API");
		}

		// Get all Vulkan-supporting devices
		std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
		vkEnumeratePhysicalDevices(this->vkInstance, &deviceCount, physicalDevices.data());

		std::multimap<int, VkPhysicalDevice> candidates;

		for (const auto &device : physicalDevices)
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
			this->logger->SimpleLog(Logging::LogLevel::Exception, LOGPFX_CURRENT "No suitable physical device found, fatal error");
			throw std::runtime_error("FATAL! No physical device found");
		}

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(this->vkPhysicalDevice, &deviceProperties);
		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Found a capable physical device: '%s'", deviceProperties.deviceName);
	}

	bool VulkanDeviceManager::checkVulkanValidationLayers()
	{
		// Get supported validation layer count
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		// Get all validation layers supported
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		// Check if any of the supported validation layers feature the ones we want to enable
		for (const char *layerName : this->vkValidationLayers)
		{
			bool layerFound = false;

			for (const auto &layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			// If none of those we want are supported, return false
			if (!layerFound)
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
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

		// Indices of which queue families we're going to use
		std::set<uint32_t> uniqueQueueFamilies = {this->graphicsQueueIndices.graphicsFamily.value(), this->graphicsQueueIndices.presentFamily.value()};

		// Fill queueCreateInfos
		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceDescriptorIndexingFeatures deviceDescriptorIndexingFeatures{};
		deviceDescriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
		deviceDescriptorIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;

		VkPhysicalDeviceRobustness2FeaturesEXT deviceRobustnessFeatures{};
		deviceRobustnessFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
		deviceRobustnessFeatures.nullDescriptor = VK_TRUE;
		deviceRobustnessFeatures.pNext = &deviceDescriptorIndexingFeatures;

		VkPhysicalDeviceFeatures2 deviceFeatures2{};
		deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		vkGetPhysicalDeviceFeatures2(this->vkPhysicalDevice, &deviceFeatures2);
		deviceFeatures2.pNext = &deviceRobustnessFeatures;

		// Logical device creation information
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.enabledExtensionCount = static_cast<uint32_t>(this->deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = this->deviceExtensions.data();
		createInfo.pNext = &deviceFeatures2;

		// Set logical device extensions
		createInfo.enabledExtensionCount = static_cast<uint32_t>(this->deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = this->deviceExtensions.data();

		// Again set validation layers, this part is apparently ignored by modern drivers
		if (this->enableVkValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(this->vkValidationLayers.size());
			createInfo.ppEnabledLayerNames = this->vkValidationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		// Create logical device
		VkResult result = vkCreateDevice(this->vkPhysicalDevice, &createInfo, nullptr, &this->vkLogicalDevice);
		if (result != VK_SUCCESS)
		{
			this->logger->SimpleLog(Logging::LogLevel::Error, LOGPFX_CURRENT "Vulkan logical device creation failed with %u code", result);
			throw std::runtime_error("Vulkan: failed to create logical device!");
		}

		// Get queue handles
		vkGetDeviceQueue(this->vkLogicalDevice, this->graphicsQueueIndices.graphicsFamily.value(), 0, &this->vkGraphicsQueue);
		vkGetDeviceQueue(this->vkLogicalDevice, this->graphicsQueueIndices.presentFamily.value(), 0, &this->vkPresentQueue);
	}

#pragma endregion

#pragma region Internal device functions

	SwapChainSupportDetails VulkanDeviceManager::queryVulkanSwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->vkSurface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->vkSurface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->vkSurface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->vkSurface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->vkSurface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	bool VulkanDeviceManager::checkVulkanDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto &extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		for (const auto &extension : requiredExtensions)
		{
			this->logger->SimpleLog(Logging::LogLevel::Warning, LOGPFX_CURRENT "Unsupported extension: %s", extension.c_str());
		}

		return requiredExtensions.empty();
	}

	QueueFamilyIndices VulkanDeviceManager::findVulkanQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto &queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, this->vkSurface, &presentSupport);

			if (presentSupport)
			{
				indices.presentFamily = i;
			}

			i++;
		}

		return indices;
	}

	int VulkanDeviceManager::checkVulkanPhysicalDeviceScore(VkPhysicalDevice device)
	{
		// Physical device properties
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		// Physical device optional features
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		int score = 0;

		// Discrete GPUs have a significant performance advantage
		switch (deviceProperties.deviceType)
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
		}

		QueueFamilyIndices indices = this->findVulkanQueueFamilies(device);

		// Return 0 (unsuitable) if some of the required queues aren't supported
		if (!indices.isComplete())
		{
			this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Found device '%s', unsuitable, required queue types unsupported", deviceProperties.deviceName, score);
			return 0;
		}
		else if (!this->checkVulkanDeviceExtensionSupport(device))
		{
			this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Found device '%s', unsuitable, cause: required extensions unsupported", deviceProperties.deviceName);
			return 0;
		}
		else if (!deviceFeatures.samplerAnisotropy)
		{
			this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Found device '%s', unsuitable, cause: anisotropy unsupported", deviceProperties.deviceName);
			return 0;
		}

		bool swapChainAdequate = false;
		SwapChainSupportDetails swapChainSupport = this->queryVulkanSwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();

		if (!swapChainAdequate)
		{
			this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Found device '%s', unsuitable, cause: inadequate or no swap chain support", deviceProperties.deviceName);
			return 0;
		}

		this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Found device '%s', suitability %u", deviceProperties.deviceName, score);

		return score;
	}

	VkSampleCountFlagBits VulkanDeviceManager::getMaxUsableSampleCount()
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(this->vkPhysicalDevice, &physicalDeviceProperties);

		VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
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

}