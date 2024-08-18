#include "Rendering/Vulkan/Wrappers/Instance.hpp"

#include "Rendering/Vulkan/Wrappers/CommandPool.hpp"
#include "Rendering/Vulkan/Wrappers/DeviceScore.hpp"
#include "Rendering/Vulkan/Wrappers/MemoryAllocator.hpp"
#include "Rendering/Vulkan/Wrappers/Surface.hpp"

#include "GLFW/glfw3.h"
#include "Logging.hpp"
#include "vulkan/vulkan_core.h"

#include <cstring>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_VULKAN_INSTANCE
#include "Logging/LoggingPrefix.hpp" // IWYU pragma: keep

namespace Engine::Rendering::Vulkan
{
#pragma region Internal static

	// NOLINTBEGIN(readability-identifier-naming)
	// these functions do not follow our naming conventions
	// their names are as specified by Vulkan
	//
	VKAPI_ATTR static VkBool32 VKAPI_CALL VulkanDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	)
	{
		(void)(messageType);

		if (auto locked_logger = (static_cast<InternalEngineObject*>(pUserData))->GetLogger().lock())
		{
			// Log as error only if error bit set
			Logging::LogLevel log_level = (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0
											  ? Logging::LogLevel::Error
											  : Logging::LogLevel::Warning;

			locked_logger
				->SimpleLog(log_level, LOGPFX_CURRENT "Vulkan validation layer reported:\n%s", pCallbackData->pMessage);
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

		return VK_ERROR_EXTENSION_NOT_PRESENT;
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
	//
	// NOLINTEND(readability-identifier-naming)

	[[noreturn]] static void GlfwErrorCallback(int error, const char* description)
	{
		using namespace std::string_literals;

		throw std::runtime_error("GLFW error handler callback called! Code: '"s.append(std::to_string(error))
									 .append("'; description: '")
									 .append(description)
									 .append("'"));
	}

#pragma endregion

#pragma region Public

	Instance::Instance(SupportsLogging::logger_t with_logger, const WindowParams&& with_window_parameters)
		: SupportsLogging(std::move(with_logger))
	{
		if (glfwInit() == GLFW_FALSE)
		{
			throw std::runtime_error("GLFW returned GLFW_FALSE on glfwInit!");
		}
		glfwSetErrorCallback(GlfwErrorCallback);

		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "GLFW initialized");

		this->InitInstance();

		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Instance initialized");

		window =
			new Window(this, with_window_parameters.size, with_window_parameters.title, with_window_parameters.hints);

		this->InitDevice();
		logical_device = new LogicalDevice(this, window->GetSurface());

		memory_allocator = new MemoryAllocator(this, *logical_device);

		command_pool = new CommandPool(this);

		window->CreateSwapchain();
	}

	Instance::~Instance()
	{
		this->logical_device->WaitDeviceIdle();

		delete window;

		delete command_pool;

		delete memory_allocator;

		delete logical_device;

		if (enable_vk_validation_layers)
		{
			DestroyDebugUtilsMessengerEXT(native_handle, debug_messenger, nullptr);
		}

		glfwTerminate();
	}

#pragma endregion

#pragma region Private

	const std::vector<const char*> Instance::validation_layers = {
		"VK_LAYER_KHRONOS_validation",
	};

	void Instance::InitInstance()
	{
		// NOLINTBEGIN(*old-style-cast) Suppress warnings for VK_API macros
		//
		// Application information
		VkApplicationInfo app_info{};
		app_info.sType				= VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName	= "An unknown CMEP application"; // TODO: this->windowTitle.c_str();
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pEngineName		= "CMEP EngineCore";
		app_info.engineVersion		= VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion			= VK_API_VERSION_1_1;
		//
		// NOLINTEND(*old-style-cast)

		// Check validation layer support
		if (enable_vk_validation_layers && !CheckVulkanValidationLayers())
		{
			throw std::runtime_error("Validation layers requested but unsupported!");
		}

		// Vulkan instance information
		VkInstanceCreateInfo create_info{};
		create_info.sType			 = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pApplicationInfo = &app_info;

		// Get extensions required by GLFW
		uint32_t glfw_extension_count = 0;
		const char** glfw_extensions  = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

		// Get our required extensions
		std::vector<const char*> vk_extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

		// Enable validation layer extension if it's a debug build
		if (enable_vk_validation_layers)
		{
			vk_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		// Add the required extensions
		create_info.enabledExtensionCount	= static_cast<uint32_t>(vk_extensions.size());
		create_info.ppEnabledExtensionNames = vk_extensions.data();

		// Enable validation layers if it's a debug build
		if (enable_vk_validation_layers)
		{
			create_info.enabledLayerCount	= static_cast<uint32_t>(validation_layers.size());
			create_info.ppEnabledLayerNames = validation_layers.data();
		}
		else
		{
			// Or else we don't enable any layers
			create_info.enabledLayerCount = 0;
		}

		// Create an instance
		if (vkCreateInstance(&create_info, nullptr, &(this->native_handle)) != VK_SUCCESS)
		{
			throw std::runtime_error("Could not create Vulkan instance");
		}
		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Created a Vulkan instance");

		// If it's a debug build, add a debug callback to Vulkan
		if (enable_vk_validation_layers)
		{
			this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Creating debug messenger");
			VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info{};
			debug_messenger_create_info.sType			= VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debug_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
														  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
														  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debug_messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
													  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
													  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debug_messenger_create_info.pfnUserCallback = VulkanDebugCallback;
			debug_messenger_create_info.pUserData		= this;

			if (CreateDebugUtilsMessengerEXT(
					this->native_handle,
					&debug_messenger_create_info,
					nullptr,
					&(this->debug_messenger)
				) != VK_SUCCESS)
			{
				throw std::runtime_error("Could not create debug messenger!");
			}

			this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Created debug messenger");
		}
	}

	bool Instance::CheckVulkanValidationLayers()
	{
		// Get supported validation layer count
		uint32_t layer_count;
		vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

		// Get all validation layers supported
		std::vector<VkLayerProperties> available_layers(layer_count);
		vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

		// Check if any of the supported validation layers feature the ones we want to enable
		for (const char* layer_name : validation_layers)
		{
			bool layer_found = false;

			for (const auto& layer_properties : available_layers)
			{
				if (std::strcmp(layer_name, layer_properties.layerName) == 0)
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

	void Instance::InitDevice()
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Initializing vulkan device");

		// Get physical device count
		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(native_handle, &device_count, nullptr);

		// Check if there are any Vulkan-supporting devices
		if (device_count == 0)
		{
			throw std::runtime_error("Found no device supporting the Vulkan API");
		}

		// Get all Vulkan-supporting devices
		std::vector<VkPhysicalDevice> physical_devices(device_count);
		vkEnumeratePhysicalDevices(native_handle, &device_count, physical_devices.data());

		// Sorted vector of all devices
		std::vector<DeviceScore> candidates;
		candidates.reserve(physical_devices.size());
		for (const auto& device : physical_devices)
		{
			auto score = DeviceScore(device, window->GetSurface());

			this->logger->SimpleLog(
				Logging::LogLevel::Debug2,
				LOGPFX_CURRENT "Found device '%s' %u %s %s",
				score.device_scored.GetDeviceName().c_str(),
				score.preference_score,
				score ? "suitable" : "unsuitable",
				score ? "" : score.unsupported_reason.data()
			);

			if (score)
			{
				candidates.emplace_back(score);
			}
		}

		std::sort(candidates.begin(), candidates.end());

		for (const auto& candidate : candidates)
		{
			if (candidate)
			{
				this->physical_device = candidate.device_scored;
				this->msaa_samples	  = GetMaxUsableSampleCount(physical_device);

				this->logger->SimpleLog(
					Logging::LogLevel::Info,
					LOGPFX_CURRENT "Found a capable physical device: '%s'",
					this->physical_device.GetDeviceName().c_str()
				);
				this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Using MSAAx%u", this->msaa_samples);

				return;
			}
		}

		throw std::runtime_error("No physical device found!");
	}

	VkSampleCountFlagBits Instance::GetMaxUsableSampleCount(VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties physical_device_properties;
		vkGetPhysicalDeviceProperties(device, &physical_device_properties);

		// Check which sample counts are supported by the framebuffers
		VkSampleCountFlags counts = physical_device_properties.limits.framebufferColorSampleCounts &
									physical_device_properties.limits.framebufferDepthSampleCounts;

		// VkSampleCountFlagBits is a bitfield
		// where each bit represents a single choice.
		// We can therefore iteratively right-shift it
		// until the bit matches one enabled in VkSampleCountFlags
		// (equal to a chain of if/else)
		//
		uint16_t bit_val = VK_SAMPLE_COUNT_64_BIT;
		while (bit_val > 0)
		{
			// Try if this bit is enabled (count supported)
			if ((counts & bit_val) != 0)
			{
				return static_cast<VkSampleCountFlagBits>(bit_val);
			}

			bit_val >>= 1;
		}

		return VK_SAMPLE_COUNT_1_BIT;
	}

#pragma endregion
} // namespace Engine::Rendering::Vulkan
