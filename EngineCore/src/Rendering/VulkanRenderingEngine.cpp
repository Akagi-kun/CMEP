#include <algorithm>
#include <set>
#include <fstream>

#include "Rendering/VulkanRenderingEngine.hpp"

#include "Logging/Logging.hpp"

namespace Engine::Rendering
{
	static VKAPI_ATTR VkBool32 VKAPI_CALL vulcanDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Warning, "Vulcan validation layer reported: %s", pCallbackData->pMessage);

		return VK_FALSE;
	}

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}

	std::vector<char> VulkanRenderingEngine::readShaderFile(std::string path)
	{
		std::ifstream file(path, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto app = reinterpret_cast<VulkanRenderingEngine*>(glfwGetWindowUserPointer(window));
		app->SignalFramebufferResizeGLFW();
	}

////////////////////////////////////////////////////////////////////////
///////////////////////    Runtime functions    ////////////////////////
////////////////////////////////////////////////////////////////////////

	int VulkanRenderingEngine::checkVulkanPhysicalDeviceScore(VkPhysicalDevice device)
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
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2, "Found device '%s', unsuitable, required queue types unsupported", deviceProperties.deviceName, score);
			return 0;
		}
		else if (!this->checkVulkanDeviceExtensionSupport(device))
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2, "Found device '%s', unsuitable, cause: required extensions unsupported", deviceProperties.deviceName);
			return 0;
		}
		else if (!deviceFeatures.samplerAnisotropy)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2, "Found device '%s', unsuitable, cause: anisotropy unsupported", deviceProperties.deviceName);
			return 0;
		}

		bool swapChainAdequate = false;
		SwapChainSupportDetails swapChainSupport = this->queryVulkanSwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();

		if (!swapChainAdequate)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2, "Found device '%s', unsuitable, cause: inadequate or no swap chain support", deviceProperties.deviceName);
			return 0;
		}

		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2, "Found device '%s', suitability %u", deviceProperties.deviceName, score);

		return score;
	}

	SwapChainSupportDetails VulkanRenderingEngine::queryVulkanSwapChainSupport(VkPhysicalDevice device)
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

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->vkSurface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	bool VulkanRenderingEngine::checkVulkanDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		for (const auto& extension : requiredExtensions) {
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Warning, "Unsupported extension: %s", extension.c_str());
		}

		return requiredExtensions.empty();
	}

	QueueFamilyIndices VulkanRenderingEngine::findVulkanQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, this->vkSurface, &presentSupport);
			
			if (presentSupport) {
				indices.presentFamily = i;
			}

			i++;
		}

		return indices;
	}
	
	void VulkanRenderingEngine::recordVulkanCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Exception, "Failed to begin recording command buffer");
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = this->vkRenderPass;
		renderPassInfo.framebuffer = this->vkSwapChainFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = this->vkSwapChainExtent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.1f, 0.1f, 0.1f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(this->vkSwapChainExtent.width);
		viewport.height = static_cast<float>(this->vkSwapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = this->vkSwapChainExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphicsPipelineDefault->vkPipelineLayout, 0, 1, &this->graphicsPipelineDefault->vkDescriptorSets[currentFrame], 0, nullptr);
		//this->SelectCurrentTopology(commandBuffer, VULKAN_RENDERING_ENGINE_TOPOLOGY_TRIANGLE_LIST);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphicsPipelineDefault->pipeline);
		if (this->external_callback)
		{
			this->external_callback(commandBuffer, currentFrame);
		}

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Exception, "Failed to record command buffer");
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	VkSurfaceFormatKHR VulkanRenderingEngine::chooseVulkanSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Warning, "Unpreferred swap surface format selected");
		return availableFormats[0];
	}

	VkPresentModeKHR VulkanRenderingEngine::chooseVulkanSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Warning, "Unpreferred swap present mode selected");
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VulkanRenderingEngine::chooseVulkanSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}
	
	uint32_t VulkanRenderingEngine::findVulkanMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(this->vkPhysicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Exception, "Failed to find required memory type");
		throw std::runtime_error("failed to find required memory type!");
	}

	VkImageView VulkanRenderingEngine::createVulkanImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		if (vkCreateImageView(this->vkLogicalDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}

		return imageView;
	}

	VkFormat VulkanRenderingEngine::findVulkanSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(this->vkPhysicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Exception, "Failed to find supported vkFormat");
		throw std::runtime_error("failed to find supported vkFormat!");
	}

	VkFormat VulkanRenderingEngine::findVulkanSupportedDepthFormat()
	{
		return this->findVulkanSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	bool VulkanRenderingEngine::doesVulkanFormatHaveStencilComponent(VkFormat format)
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

////////////////////////////////////////////////////////////////////////
////////////////////////    Init functions    //////////////////////////
////////////////////////////////////////////////////////////////////////

	void VulkanRenderingEngine::createVulkanLogicalDevice()
	{
		this->graphicsQueueIndices = this->findVulkanQueueFamilies(this->vkPhysicalDevice);

		// Vector of queue creation structs
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

		// Indices of which queue families we're going to use
		std::set<uint32_t> uniqueQueueFamilies = { this->graphicsQueueIndices.graphicsFamily.value(), this->graphicsQueueIndices.presentFamily.value() };

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
		else {
			createInfo.enabledLayerCount = 0;
		}

		// Create logical device
		VkResult result = vkCreateDevice(this->vkPhysicalDevice, &createInfo, nullptr, &this->vkLogicalDevice);
		if (result != VK_SUCCESS) {
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "Vulkan logical device creation failed with %u code", result);
			throw std::runtime_error("Vulkan: failed to create logical device!");
		}

		// Get queue handles
		vkGetDeviceQueue(this->vkLogicalDevice, this->graphicsQueueIndices.graphicsFamily.value(), 0, &this->vkGraphicsQueue);
		vkGetDeviceQueue(this->vkLogicalDevice, this->graphicsQueueIndices.presentFamily.value(), 0, &this->vkPresentQueue);
	}

	void VulkanRenderingEngine::createVulkanSurface()
	{
		if (glfwCreateWindowSurface(this->vkInstance, this->window, nullptr, &this->vkSurface) != VK_SUCCESS) {
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "Vulkan surface creation failed");
			throw std::runtime_error("failed to create window surface!");
		}
		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2, "Created glfw window surface");
	}

	bool VulkanRenderingEngine::checkVulkanValidationLayers()
	{
		// Get supported validation layer count
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		// Get all validation layers supported
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		// Check if any of the supported validation layers feature the ones we want to enable
		for (const char* layerName : this->vkValidationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			// If none of those we want are supported, return false
			if (!layerFound) {
				return false;
			}
		}

		return true;
	}
	
	void VulkanRenderingEngine::initVulkanInstance()
	{
		// Application information
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = this->windowTitle.c_str();
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "CMEP";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_1;

		// Check validation layer support
		if (this->enableVkValidationLayers && !this->checkVulkanValidationLayers())
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "Validation layer support requested but not allowed!");
		}

		// Vulkan instance information
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// Get extensions required by GLFW
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		// Get our required extensions
		std::vector<const char*> vkExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

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
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "Could not create Vulkan instance");
			return;
		}
		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Info, "Created a Vulkan instance");

		// If it's a debug build, add a debug callback to Vulkan
		if (this->enableVkValidationLayers)
		{
			VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
			debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debugMessengerCreateInfo.pfnUserCallback = vulcanDebugCallback;
			debugMessengerCreateInfo.pUserData = nullptr; // User data passed to callback, unused

			if (CreateDebugUtilsMessengerEXT(this->vkInstance, &debugMessengerCreateInfo, nullptr, &(this->vkDebugMessenger)) != VK_SUCCESS)
			{
				Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "Could not create a debug messenger");
			}
		}
	}

	void VulkanRenderingEngine::initVulkanDevice()
	{
		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2, "Initializing vulkan device");
		// Get physical device count
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(this->vkInstance, &deviceCount, nullptr);

		// Check if there are any Vulkan-supporting devices
		if (deviceCount == 0)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "Found no device supporting the Vulcan API");
		}

		// Get all Vulkan-supporting devices
		std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
		vkEnumeratePhysicalDevices(this->vkInstance, &deviceCount, physicalDevices.data());

		std::multimap<int, VkPhysicalDevice> candidates;

		for (const auto& device : physicalDevices)
		{
			int score = this->checkVulkanPhysicalDeviceScore(device);
			candidates.insert(std::make_pair(score, device));
		}

		// Check if the best candidate is suitable at all
		if (candidates.rbegin()->first > 0)
		{
			this->vkPhysicalDevice = candidates.rbegin()->second;
		}

		if (this->vkPhysicalDevice == VK_NULL_HANDLE)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Exception, "No suitable physical device found, fatal error");
			throw std::runtime_error("FATAL! No physical device found");
		}

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(this->vkPhysicalDevice, &deviceProperties);
		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Info, "Found a capable physical device: '%s'", deviceProperties.deviceName);
	}

	void VulkanRenderingEngine::createVulkanSwapChain()
	{
		// Get device and surface Swap Chain capabilities
		SwapChainSupportDetails swapChainSupport = this->queryVulkanSwapChainSupport(this->vkPhysicalDevice);
		
		// Get the info out of the capabilities
		VkSurfaceFormatKHR surfaceFormat = this->chooseVulkanSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = this->chooseVulkanSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = this->chooseVulkanSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug1, "Using maxImageCount capability, GPU support limited");
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = this->vkSurface;

		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug1, "Creating Vulkan swap chain with %u images", imageCount);

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		uint32_t queueFamilyIndices[] = { this->graphicsQueueIndices.graphicsFamily.value(), this->graphicsQueueIndices.presentFamily.value() };

		if (this->graphicsQueueIndices.graphicsFamily != this->graphicsQueueIndices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(this->vkLogicalDevice, &createInfo, nullptr, &(this->vkSwapChain)) != VK_SUCCESS) {
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "Vulkan swap chain creation failed");
			throw std::runtime_error("failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR(this->vkLogicalDevice, this->vkSwapChain, &imageCount, nullptr);
		this->vkSwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(this->vkLogicalDevice, this->vkSwapChain, &imageCount, this->vkSwapChainImages.data());

		this->vkSwapChainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;
		this->vkSwapChainExtent = extent;

	}

	void VulkanRenderingEngine::recreateVulkanSwapChain()
	{
		// If window is minimized, wait for it to show up again
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(this->vkLogicalDevice);

		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug1, "Recreating vulkan swap chain");

		// Clean up old swap chain
		this->cleanupVulkanSwapChain();

		// Create a new swap chain
		this->createVulkanSwapChain();
		this->createVulkanSwapChainViews();
		this->createVulkanFramebuffers();
	}

	void VulkanRenderingEngine::cleanupVulkanSwapChain()
	{
		for (auto framebuffer : this->vkSwapChainFramebuffers) {
			vkDestroyFramebuffer(this->vkLogicalDevice, framebuffer, nullptr);
		}
		
		for (auto imageView : this->vkSwapChainImageViews)
		{
			vkDestroyImageView(this->vkLogicalDevice, imageView, nullptr);
		}

		vkDestroySwapchainKHR(this->vkLogicalDevice, this->vkSwapChain, nullptr);
	}

	void VulkanRenderingEngine::createVulkanSwapChainViews()
	{
		this->vkSwapChainImageViews.resize(this->vkSwapChainImages.size());

		for (size_t i = 0; i < this->vkSwapChainImages.size(); i++)
		{
			this->vkSwapChainImageViews[i] = this->createVulkanImageView(this->vkSwapChainImages[i], VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
		}
	}

	VkShaderModule VulkanRenderingEngine::createVulkanShaderModule(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(this->vkLogicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "Vulkan failed creating shader module");
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}

	void VulkanRenderingEngine::createVulkanDefaultGraphicsPipeline()
	{
		VulkanPipelineSettings pipeline_settings = this->getVulkanDefaultPipelineSettings();
		pipeline_settings.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		pipeline_settings.descriptorLayoutSettings.binding.push_back(0);
		pipeline_settings.descriptorLayoutSettings.descriptorCount.push_back(1);
		pipeline_settings.descriptorLayoutSettings.types.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		pipeline_settings.descriptorLayoutSettings.stageFlags.push_back(VK_SHADER_STAGE_VERTEX_BIT);

		this->graphicsPipelineDefault = this->createVulkanPipeline(pipeline_settings, "game/shaders/vulkan/default_vert.spv", "game/shaders/vulkan/default_frag.spv");
	}

	void VulkanRenderingEngine::createVulkanRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = this->vkSwapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = this->findVulkanSupportedDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 2;
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(this->vkLogicalDevice, &renderPassInfo, nullptr, &this->vkRenderPass) != VK_SUCCESS)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "Vulkan failed creating render pass");
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void VulkanRenderingEngine::createVulkanFramebuffers()
	{
		this->vkSwapChainFramebuffers.resize(this->vkSwapChainImageViews.size());

		for (size_t i = 0; i < vkSwapChainImageViews.size(); i++) {
			VkImageView attachments[] = {
				this->vkSwapChainImageViews[i],
				this->vkDepthBuffer->imageView
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = this->vkRenderPass;
			framebufferInfo.attachmentCount = 2;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = this->vkSwapChainExtent.width;
			framebufferInfo.height = this->vkSwapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(this->vkLogicalDevice, &framebufferInfo, nullptr, &this->vkSwapChainFramebuffers[i]) != VK_SUCCESS)
			{
				Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "Vulkan failed creating framebuffers");
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	void VulkanRenderingEngine::createVulkanCommandPools()
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = this->graphicsQueueIndices.graphicsFamily.value();
	
		if (vkCreateCommandPool(this->vkLogicalDevice, &poolInfo, nullptr, &(this->vkCommandPool)) != VK_SUCCESS)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "Vulkan failed creating command pools");
			throw std::runtime_error("failed to create command pool!");
		}
	}

	void VulkanRenderingEngine::createVulkanCommandBuffers()
	{
		vkCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = this->vkCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)vkCommandBuffers.size();

		if (vkAllocateCommandBuffers(this->vkLogicalDevice, &allocInfo, this->vkCommandBuffers.data()) != VK_SUCCESS)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "Vulkan failed creating command pools");
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void VulkanRenderingEngine::createVulkanSyncObjects()
	{
		this->imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		this->renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		this->inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (vkCreateSemaphore(this->vkLogicalDevice, &semaphoreInfo, nullptr, &(this->imageAvailableSemaphores[i])) != VK_SUCCESS ||
				vkCreateSemaphore(this->vkLogicalDevice, &semaphoreInfo, nullptr, &(this->renderFinishedSemaphores[i])) != VK_SUCCESS ||
				vkCreateFence(this->vkLogicalDevice, &fenceInfo, nullptr, &(this->inFlightFences[i])) != VK_SUCCESS)
			{

				Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "Vulkan failed creating sync objects");
				throw std::runtime_error("failed to create sync objects!");
			}
		}
	}

	void VulkanRenderingEngine::createVulkanDepthResources()
	{
		VkFormat depthFormat = this->findVulkanSupportedDepthFormat();

		this->vkDepthBuffer = this->createVulkanImage(this->vkSwapChainExtent.width, this->vkSwapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		this->vkDepthBuffer->imageView = this->createVulkanImageView(this->vkDepthBuffer->image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	}

////////////////////////////////////////////////////////////////////////
///////////////////////    Public Interface    /////////////////////////
////////////////////////////////////////////////////////////////////////

	void VulkanRenderingEngine::cleanup()
	{
		this->cleanupVulkanSwapChain();

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(this->vkLogicalDevice, renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(this->vkLogicalDevice, imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(this->vkLogicalDevice, inFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(this->vkLogicalDevice, this->vkCommandPool, nullptr);

		this->cleanupVulkanPipeline(this->graphicsPipelineDefault);

		vkDestroyRenderPass(this->vkLogicalDevice, this->vkRenderPass, nullptr);
		
		vkDestroySurfaceKHR(this->vkInstance, this->vkSurface, nullptr);
		vkDestroyDevice(this->vkLogicalDevice, nullptr);
		if (this->enableVkValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT(this->vkInstance, this->vkDebugMessenger, nullptr);
		}
		vkDestroyInstance(this->vkInstance, nullptr);

		// Clean up GLFW
		glfwDestroyWindow(this->window);
		glfwTerminate();
	}

	void VulkanRenderingEngine::init(unsigned int xsize, unsigned int ysize, std::string title)
	{
		this->windowX = xsize;
		this->windowY = ysize;
		this->windowTitle = title;

		// Initialize GLFW
		if (!glfwInit())
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "glfwInit returned 0!");
			exit(1);
		}
		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Info, "GLFW initialized");

		// Create a GLFW window
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		this->window = glfwCreateWindow(this->windowX, this->windowY, this->windowTitle.c_str(), NULL, NULL);
		glfwSetWindowTitle(this->window, this->windowTitle.c_str());
		glfwSetWindowSize(this->window, this->windowX, this->windowY);
		glfwSetWindowUserPointer(this->window, this);
		glfwSetFramebufferSizeCallback(this->window, framebufferResizeCallback);

		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug1, "%u vulkan extensions supported", extensionCount);

		// Set up our vulkan rendering stack
		this->initVulkanInstance();
		this->createVulkanSurface();
		this->initVulkanDevice();
		this->createVulkanLogicalDevice();
		this->createVulkanSwapChain();
		this->createVulkanSwapChainViews();
		this->createVulkanRenderPass();
		this->createVulkanDefaultGraphicsPipeline();
		this->createVulkanCommandPools();
		this->createVulkanCommandBuffers();
		this->createVulkanDepthResources();
		this->createVulkanFramebuffers();
		this->createVulkanSyncObjects();
	}

	void VulkanRenderingEngine::drawFrame()
	{
		vkWaitForFences(this->vkLogicalDevice, 1, &this->inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult acquire_result = vkAcquireNextImageKHR(this->vkLogicalDevice, this->vkSwapChain, UINT64_MAX, this->imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR || this->framebufferResized || acquire_result == VK_SUBOPTIMAL_KHR)
		{
			this->currentFrame = (this->currentFrame + 1) % this->MAX_FRAMES_IN_FLIGHT;
			this->framebufferResized = false;

			this->recreateVulkanSwapChain();
			
			return;
		}
		else if (acquire_result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		vkResetFences(this->vkLogicalDevice, 1, &this->inFlightFences[currentFrame]);

		vkResetCommandBuffer(this->vkCommandBuffers[currentFrame], 0);
		this->recordVulkanCommandBuffer(this->vkCommandBuffers[currentFrame], imageIndex);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		// Wait semaphores
		VkSemaphore waitSemaphores[] = { this->imageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &this->vkCommandBuffers[currentFrame];

		// Signal semaphores
		VkSemaphore signalSemaphores[] = { this->renderFinishedSemaphores[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(this->vkGraphicsQueue, 1, &submitInfo, this->inFlightFences[currentFrame]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		this->currentFrame = (this->currentFrame + 1) % this->MAX_FRAMES_IN_FLIGHT;

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { this->vkSwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // Optional

		vkQueuePresentKHR(this->vkPresentQueue, &presentInfo);
	}

	GLFWwindowData const VulkanRenderingEngine::GetWindow()
	{
		GLFWwindowData data{};
		data.window = this->window;
		data.windowX = this->windowX;
		data.windowY = this->windowY;
		data.windowTitle = this->windowTitle;

		return data;
	}

	const uint32_t VulkanRenderingEngine::GetMaxFramesInFlight()
	{
		return this->MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanRenderingEngine::SetRenderCallback(std::function<void(VkCommandBuffer, uint32_t)> callback)
	{
		this->external_callback = callback;
	}

	void VulkanRenderingEngine::SignalFramebufferResizeGLFW()
	{
		this->framebufferResized = true;
	}

	VkCommandBuffer VulkanRenderingEngine::beginVulkanSingleTimeCommandsCommandBuffer()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = this->vkCommandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(this->vkLogicalDevice, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void VulkanRenderingEngine::endVulkanSingleTimeCommandsCommandBuffer(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(this->vkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(this->vkGraphicsQueue);

		vkFreeCommandBuffers(this->vkLogicalDevice, this->vkCommandPool, 1, &commandBuffer);
	}

	VkDevice VulkanRenderingEngine::GetLogicalDevice()
	{
		return this->vkLogicalDevice;
	}

	// Pipelines
	VulkanPipelineSettings VulkanRenderingEngine::getVulkanDefaultPipelineSettings()
	{
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)this->vkSwapChainExtent.width;
		viewport.height = (float)this->vkSwapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = this->vkSwapChainExtent;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f; // Optional
		depthStencil.maxDepthBounds = 1.0f; // Optional
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {}; // Optional
		depthStencil.back = {}; // Optional

		VulkanPipelineSettings default_settings{};
		default_settings.inputAssembly = inputAssembly;
		default_settings.viewport = viewport;
		default_settings.scissor = scissor;
		default_settings.viewportState = viewportState;
		default_settings.rasterizer = rasterizer;
		default_settings.multisampling = multisampling;
		default_settings.colorBlendAttachment = colorBlendAttachment;
		default_settings.colorBlending = colorBlending;
		default_settings.depthStencil = depthStencil;
		default_settings.descriptorLayoutSettings = {};

		return default_settings;
	}

	VulkanPipeline* VulkanRenderingEngine::createVulkanPipelineFromPrealloc(VulkanPipeline* pipeline, VulkanPipelineSettings settings, std::string vert_path, std::string frag_path)
	{
		if (this->leakPipelineCounter > 300)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Warning, "Currently allocated pipeline count %u, possibly leaking pipelines", this->leakPipelineCounter);
		}

		settings.colorBlending.pAttachments = &settings.colorBlendAttachment;

		auto vertShaderCode = VulkanRenderingEngine::readShaderFile(vert_path);
		auto fragShaderCode = VulkanRenderingEngine::readShaderFile(frag_path);

		VkShaderModule vertShaderModule = this->createVulkanShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = this->createVulkanShaderModule(fragShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		auto bindingDescription = RenderingVertex::getBindingDescription();
		auto attributeDescriptions = RenderingVertex::getAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		this->createVulkanDescriptorSetLayout(pipeline, settings.descriptorLayoutSettings);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &pipeline->vkDescriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		if (vkCreatePipelineLayout(this->vkLogicalDevice, &pipelineLayoutInfo, nullptr, &(pipeline->vkPipelineLayout)) != VK_SUCCESS)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "Vulkan failed creating graphics pipeline layout");
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &settings.inputAssembly;
		pipelineInfo.pViewportState = &settings.viewportState;
		pipelineInfo.pRasterizationState = &settings.rasterizer;
		pipelineInfo.pMultisampleState = &settings.multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &settings.colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = pipeline->vkPipelineLayout;
		pipelineInfo.pDepthStencilState = &settings.depthStencil;
		pipelineInfo.renderPass = this->vkRenderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		if (vkCreateGraphicsPipelines(this->vkLogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &(pipeline->pipeline)) != VK_SUCCESS)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Exception, "Vulkan failed creating triangle graphics pipeline");
			throw std::runtime_error("failed to create triangle graphics pipeline!");
		}

		vkDestroyShaderModule(this->vkLogicalDevice, fragShaderModule, nullptr);
		vkDestroyShaderModule(this->vkLogicalDevice, vertShaderModule, nullptr);

		this->createVulkanUniformBuffers(pipeline);
		this->createVulkanDescriptorPool(pipeline, settings.descriptorLayoutSettings);
		this->createVulkanDescriptorSets(pipeline, settings.descriptorLayoutSettings);

		this->leakPipelineCounter += 1;

		return pipeline;
	}

	VulkanPipeline* VulkanRenderingEngine::createVulkanPipeline(VulkanPipelineSettings settings, std::string vert_path, std::string frag_path)
	{
		VulkanPipeline* new_pipeline = new VulkanPipeline();

		this->createVulkanPipelineFromPrealloc(new_pipeline, settings, vert_path, frag_path);

		return new_pipeline;
	}

	void VulkanRenderingEngine::cleanupVulkanPipeline(VulkanPipeline* pipeline)
	{
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyBuffer(this->vkLogicalDevice, this->graphicsPipelineDefault->uniformBuffers[i]->buffer, nullptr);
			vkFreeMemory(this->vkLogicalDevice, this->graphicsPipelineDefault->uniformBuffers[i]->bufferMemory, nullptr);
			this->leakUniformBufferCounter -= 1;
		}

		vkDestroyDescriptorPool(this->vkLogicalDevice, this->graphicsPipelineDefault->vkDescriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(this->vkLogicalDevice, this->graphicsPipelineDefault->vkDescriptorSetLayout, nullptr);

		vkDestroyPipeline(this->vkLogicalDevice, pipeline->pipeline, nullptr);

		this->leakPipelineCounter -= 1;

		delete pipeline;
	}


	// Buffers
	VulkanBuffer* VulkanRenderingEngine::createVulkanVertexBufferFromData(std::vector<RenderingVertex> vertices)
	{
		VulkanBuffer* stagingBuffer{};
		VulkanBuffer* vertexBuffer{};

		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		stagingBuffer = this->createVulkanBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		
		void* data;
		vkMapMemory(this->vkLogicalDevice, stagingBuffer->bufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(this->vkLogicalDevice, stagingBuffer->bufferMemory);

		//stagingBuffer = this->createVulkanStagingBufferWithData(vertices.data(), bufferSize);

		vertexBuffer = this->createVulkanBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		this->bufferVulkanTransferCopy(stagingBuffer, vertexBuffer, bufferSize);

		this->cleanupVulkanBuffer(stagingBuffer);

		return vertexBuffer;
	}

	void VulkanRenderingEngine::createVulkanUniformBuffers(VulkanPipeline* pipeline)
	{
		if (this->leakUniformBufferCounter > 300)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Warning, "Currently allocated uniform buffer count %u, possibly leaking uniform buffers", this->leakUniformBufferCounter);
		}

		VkDeviceSize bufferSize = sizeof(glm::mat4);

		pipeline->uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			pipeline->uniformBuffers[i] = this->createVulkanBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			vkMapMemory(this->vkLogicalDevice, pipeline->uniformBuffers[i]->bufferMemory, 0, bufferSize, 0, &(pipeline->uniformBuffers[i]->mappedMemory));
			this->leakUniformBufferCounter += 1;
		}
	}

	VulkanBuffer* VulkanRenderingEngine::createVulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
	{
		VulkanBuffer* new_buffer = new VulkanBuffer();

		if(this->leakBufferCounter > 300)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Warning, "Currently allocated buffer count %u, possibly leaking buffers", this->leakBufferCounter);
		}

		// Create a buffer handle
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(this->vkLogicalDevice, &bufferInfo, nullptr, &(new_buffer->buffer)) != VK_SUCCESS)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "Vulkan failed creating buffer");
			throw std::runtime_error("failed to create buffer!");
		}

		// Get memory requiremenets of this buffer
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(this->vkLogicalDevice, new_buffer->buffer, &memRequirements);

		// Allocate the required memory
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = this->findVulkanMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(this->vkLogicalDevice, &allocInfo, nullptr, &(new_buffer->bufferMemory)) != VK_SUCCESS)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "Vulkan failed allocating buffer memory");
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		// Assign the memory to buffer
		vkBindBufferMemory(this->vkLogicalDevice, new_buffer->buffer, new_buffer->bufferMemory, 0);

		this->leakBufferCounter += 1;

		return new_buffer;
	}

	VulkanBuffer* VulkanRenderingEngine::createVulkanStagingBufferPreMapped(VkDeviceSize dataSize)
	{
		VulkanBuffer* staging_buffer;

		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2, "Creating a staging buffer with size of %u", dataSize);

		staging_buffer = this->createVulkanBuffer(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		vkMapMemory(this->vkLogicalDevice, staging_buffer->bufferMemory, 0, dataSize, 0, &staging_buffer->mappedMemory);
		
		return staging_buffer;
	}

	VulkanBuffer* VulkanRenderingEngine::createVulkanStagingBufferWithData(void* data, VkDeviceSize dataSize)
	{
		VulkanBuffer* staging_buffer;

		staging_buffer = this->createVulkanStagingBufferPreMapped(dataSize);

		memcpy(staging_buffer->mappedMemory, data, static_cast<size_t>(dataSize));

		return staging_buffer;
	}

	void VulkanRenderingEngine::bufferVulkanTransferCopy(VulkanBuffer* src, VulkanBuffer* dest, VkDeviceSize size)
	{
		VkCommandBuffer commandBuffer = this->beginVulkanSingleTimeCommandsCommandBuffer();

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, src->buffer, dest->buffer, 1, &copyRegion);

		this->endVulkanSingleTimeCommandsCommandBuffer(commandBuffer);
	}

	void VulkanRenderingEngine::cleanupVulkanBuffer(VulkanBuffer* buffer)
	{
		this->leakBufferCounter -= 1;

		vkDestroyBuffer(this->vkLogicalDevice, buffer->buffer, nullptr);
		vkFreeMemory(this->vkLogicalDevice, buffer->bufferMemory, nullptr);

		// Also delete as we use pointers
		delete buffer;
	}


	// Descriptor sets
	void VulkanRenderingEngine::createVulkanDescriptorSetLayout(VulkanPipeline* pipeline, VulkanDescriptorLayoutSettings settings)
	{
		/*VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
	*/
		std::vector<VkDescriptorSetLayoutBinding> bindings = {};
		std::vector<VkDescriptorBindingFlags> bindingFlags = {};
		for (size_t i = 0; i < settings.binding.size(); i++)
		{
			VkDescriptorSetLayoutBinding new_binding{};
			new_binding.binding = settings.binding[i];
			new_binding.descriptorCount = settings.descriptorCount[i];
			new_binding.descriptorType = settings.types[i];
			new_binding.stageFlags = settings.stageFlags[i];
			new_binding.pImmutableSamplers = nullptr;

			bindings.push_back(new_binding);

			VkDescriptorBindingFlags new_flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
			bindingFlags.push_back(new_flags);
		}

		VkDescriptorSetLayoutBindingFlagsCreateInfo layoutFlagsInfo{};
		layoutFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		layoutFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
		layoutFlagsInfo.pBindingFlags = bindingFlags.data();

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();
		layoutInfo.pNext = &layoutFlagsInfo;

		if (vkCreateDescriptorSetLayout(this->vkLogicalDevice, &layoutInfo, nullptr, &(pipeline->vkDescriptorSetLayout)) != VK_SUCCESS)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Exception, "Vulkan failed to create descriptor set layout");
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void VulkanRenderingEngine::createVulkanDescriptorPool(VulkanPipeline* pipeline, VulkanDescriptorLayoutSettings settings)
	{
		std::vector<VkDescriptorPoolSize> poolSizes{};
		/*poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);*/

		poolSizes.resize(settings.binding.size());
		for (int i = 0; i < settings.binding.size(); i++)
		{
			VkDescriptorPoolSize poolSize{};
			poolSize.type = settings.types[i];
			poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * settings.descriptorCount[i];

			poolSizes[i] = poolSize;
		}

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		if (vkCreateDescriptorPool(this->vkLogicalDevice, &poolInfo, nullptr, &(pipeline->vkDescriptorPool)) != VK_SUCCESS)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Exception, "Vulkan failed to create descriptor pool");
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void VulkanRenderingEngine::createVulkanDescriptorSets(VulkanPipeline* pipeline, VulkanDescriptorLayoutSettings settings)
	{
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, pipeline->vkDescriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pipeline->vkDescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		pipeline->vkDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		VkResult createResult{};
		if ((createResult = vkAllocateDescriptorSets(this->vkLogicalDevice, &allocInfo, pipeline->vkDescriptorSets.data())) != VK_SUCCESS)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Exception, "Vulkan failed to create descriptor sets, VkResult: %i", createResult);
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		//for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		//{
		//	VkDescriptorBufferInfo bufferInfo{};
		//	bufferInfo.buffer = pipeline->uniformBuffers[i]->buffer;
		//	bufferInfo.offset = 0;
		//	bufferInfo.range = sizeof(glm::mat4);

		//	VkWriteDescriptorSet descriptorWrite{};
		//	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//	descriptorWrite.dstSet = pipeline->vkDescriptorSets[i];
		//	descriptorWrite.dstBinding = 0;
		//	descriptorWrite.dstArrayElement = 0;
		//	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		//	descriptorWrite.descriptorCount = 1;
		//	descriptorWrite.pBufferInfo = &bufferInfo;
		//	descriptorWrite.pImageInfo = nullptr; // Optional
		//	descriptorWrite.pTexelBufferView = nullptr; // Optional

		//	vkUpdateDescriptorSets(this->vkLogicalDevice, 1, &descriptorWrite, 0, nullptr);
		//}
	}


	// Image functions
	VulkanImage* VulkanRenderingEngine::createVulkanImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
	{
		VulkanImage* new_image = new VulkanImage();

		new_image->imageFormat = format;

		if (this->leakImageCounter > 300)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Warning, "Currently allocated image count %u, possibly leaking images", this->leakImageCounter);
		}

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = static_cast<uint32_t>(width);
		imageInfo.extent.height = static_cast<uint32_t>(height);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0; // Optional

		if (vkCreateImage(this->vkLogicalDevice, &imageInfo, nullptr, &(new_image->image)) != VK_SUCCESS)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "Failed to create image");
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(this->vkLogicalDevice, new_image->image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = this->findVulkanMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(this->vkLogicalDevice, &allocInfo, nullptr, &(new_image->imageMemory)) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(this->vkLogicalDevice, new_image->image, new_image->imageMemory, 0);

		this->leakImageCounter += 1;

		return new_image;
	}

	VulkanTextureImage* VulkanRenderingEngine::createVulkanTextureImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
	{
		VulkanTextureImage* new_texture_image = new VulkanTextureImage();

		if (this->leakTextureImageCounter > 300)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Warning, "Currently allocated texture image count %u, possibly leaking texture images", this->leakTextureImageCounter);
		}

		new_texture_image->image = this->createVulkanImage(width, height, format, tiling, usage, properties);

		this->leakTextureImageCounter += 1;

		return new_texture_image;
	}

	void VulkanRenderingEngine::copyVulcanBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = this->beginVulkanSingleTimeCommandsCommandBuffer();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		this->endVulkanSingleTimeCommandsCommandBuffer(commandBuffer);
	}

	void VulkanRenderingEngine::transitionVulkanImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkCommandBuffer commandBuffer = this->beginVulkanSingleTimeCommandsCommandBuffer();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "Unsupported layout transition requested");
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		this->endVulkanSingleTimeCommandsCommandBuffer(commandBuffer);
	}

	void VulkanRenderingEngine::appendVulkanImageViewToVulkanTextureImage(VulkanTextureImage* teximage)
	{
		teximage->image->imageView = this->createVulkanImageView(teximage->image->image, teximage->image->imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	void VulkanRenderingEngine::appendVulkanSamplerToVulkanTextureImage(VulkanTextureImage* teximage)
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(this->vkPhysicalDevice, &properties);

		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(this->vkLogicalDevice, &samplerInfo, nullptr, &(teximage->textureSampler)) != VK_SUCCESS)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "Failed to create texture sampler");
			throw std::runtime_error("failed to create texture sampler!");
		}
	}
	
	void VulkanRenderingEngine::cleanupVulkanImage(VulkanImage* image)
	{
		this->leakImageCounter -= 1;

		vkDestroyImageView(this->vkLogicalDevice, image->imageView, nullptr);
		vkDestroyImage(this->vkLogicalDevice, image->image, nullptr);
		vkFreeMemory(this->vkLogicalDevice, image->imageMemory, nullptr);

		// Also delete as we use pointers
		delete image;
	}

	void VulkanRenderingEngine::cleanupVulkanTextureImage(VulkanTextureImage* image)
	{
		this->leakTextureImageCounter -= 1;

		vkDestroySampler(this->vkLogicalDevice, image->textureSampler, nullptr);
		
		this->cleanupVulkanImage(image->image);

		// Also delete as we use pointers
		delete image;
	}

}