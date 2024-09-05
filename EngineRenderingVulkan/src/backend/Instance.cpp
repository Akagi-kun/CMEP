#include "backend/Instance.hpp"

#include "Logging/Logging.hpp"

#include "backend/DeviceScore.hpp"
#include "backend/LogicalDevice.hpp"
#include "backend/MemoryAllocator.hpp"
#include "backend/PhysicalDevice.hpp"
#include "objects/CommandPool.hpp"
#include "rendering/Surface.hpp"
#include "rendering/Window.hpp"
#include "vulkan/vulkan.hpp"

#include <algorithm>
#include <cstring>
#include <format>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace Engine::Rendering::Vulkan
{
#pragma region Internal static

	// NOLINTBEGIN(readability-identifier-naming)
	// these functions do not follow our naming conventions
	// names are as specified by Vulkan
	//
	VKAPI_ATTR static VkBool32 VKAPI_CALL VulkanDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT		messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT				messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void*										pUserData
	)
	{
		(void)(messageType);

		if (auto locked_logger = (static_cast<Logging::SupportsLogging*>(pUserData))->GetLogger())
		{
			// Log as error only if error bit set
			Logging::LogLevel log_level = (messageSeverity &
										   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0
											  ? Logging::LogLevel::Error
											  : Logging::LogLevel::Warning;

			locked_logger->SimpleLog<void>(
				log_level,
				"Vulkan validation layer reported:\n%s",
				pCallbackData->pMessage
			);
		}

		return VK_FALSE;
	}
	// NOLINTEND(readability-identifier-naming)

	[[noreturn]] static void GlfwErrorCallback(int error, const char* description)
	{
		throw std::runtime_error(std::format(
			"GLFW error handler callback called! Code: '{}'; description: '{}'",
			error,
			description
		));
	}

#pragma endregion

#pragma region Public

	Instance::Instance(
		SupportsLogging::logger_t with_logger,
		const WindowParams&&	  with_window_parameters
	)
		: SupportsLogging(std::move(with_logger))
	{
		if (glfwInit() == GLFW_FALSE)
		{
			throw std::runtime_error("GLFW returned GLFW_FALSE on glfwInit!");
		}
		glfwSetErrorCallback(GlfwErrorCallback);

		this->logger->SimpleLog<decltype(this)>(Logging::LogLevel::Info, "GLFW initialized");

		// Initialize dynamic dispatcher base before all other vulkan calls
		VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

		InitInstance();

		this->logger->SimpleLog<decltype(this)>(Logging::LogLevel::Info, "Instance initialized");

		window = new Window(
			this,
			with_window_parameters.size,
			with_window_parameters.title,
			with_window_parameters.hints
		);

		InitDevice();
		logical_device = new LogicalDevice(this, window->GetSurface());

		memory_allocator = new MemoryAllocator(this, *logical_device);

		command_pool = new CommandPool(this);

		window->CreateSwapchain();
	}

	Instance::~Instance()
	{
		logical_device->waitIdle();

		delete window;

		delete command_pool;

		delete memory_allocator;

		delete logical_device;

		delete physical_device;

		glfwTerminate();
	}

#pragma endregion

#pragma region Private

	const std::vector<const char*> Instance::validation_layers = {
		"VK_LAYER_KHRONOS_validation",
	};

	void Instance::InitInstance()
	{
		// Application information
		vk::ApplicationInfo app_info("A CMEP application", 1, "CMEP", 1, vk::ApiVersion11);

		// Check validation layer support
		if (enable_vk_validation_layers && !CheckVulkanValidationLayers())
		{
			throw std::runtime_error("Validation layers requested but unsupported!");
		}

		// Get extensions required by GLFW
		uint32_t	 glfw_extension_count = 0;
		const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

		// Get our required extensions
		std::vector<const char*> instance_extensions(
			glfw_extensions,
			glfw_extensions + glfw_extension_count
		);
		std::vector<const char*> instance_layers = enable_vk_validation_layers
													   ? validation_layers
													   : std::vector<const char*>{};

		// Enable validation layer extension if it's a debug build
		if (enable_vk_validation_layers)
		{
			instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		native_handle = context.createInstance(vk::InstanceCreateInfo{
			{},
			&app_info,
			instance_layers,
			instance_extensions,
		});

		// Load instance functions in dispatcher
		VULKAN_HPP_DEFAULT_DISPATCHER.init(*native_handle);

		this->logger->SimpleLog<decltype(this)>(
			Logging::LogLevel::Info,
			"Created a Vulkan instance"
		);

		// If it's a debug build, add a debug callback to Vulkan
		if (enable_vk_validation_layers)
		{
			this->logger->SimpleLog<decltype(this)>(
				Logging::LogLevel::VerboseDebug,
				"Creating debug messenger"
			);

			vk::DebugUtilsMessengerCreateInfoEXT messenger_create_info(
				{},
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
					vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
					vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
				vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
					vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
					vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
				VulkanDebugCallback,
				this
			);

			debug_messenger = native_handle.createDebugUtilsMessengerEXT(messenger_create_info);

			this->logger->SimpleLog<decltype(this)>(
				Logging::LogLevel::VerboseDebug,
				"Created debug messenger"
			);
		}
	}

	bool Instance::CheckVulkanValidationLayers()
	{
		std::vector<vk::LayerProperties> available_layers = vk::enumerateInstanceLayerProperties();

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
		this->logger->SimpleLog<decltype(this)>(
			Logging::LogLevel::Debug,
			"Initializing vulkan device"
		);

		// Get physical devices
		std::vector<vk::raii::PhysicalDevice> physical_devices =
			native_handle.enumeratePhysicalDevices();

		// Sorted (by score) vector of all devices
		std::vector<DeviceScore> candidates;
		candidates.reserve(physical_devices.size());
		for (const auto& device : physical_devices)
		{
			auto score = DeviceScore(device, window->GetSurface());

			this->logger->SimpleLog<decltype(this)>(
				Logging::LogLevel::VerboseDebug,
				"Found device '%s' %u %s %s",
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

		if (candidates.empty())
		{
			throw std::runtime_error("No physical device found!");
		}

		std::sort(candidates.begin(), candidates.end());

		// First device has to be the "best" after sorting
		physical_device = new PhysicalDevice(candidates[0].device_scored);
		// msaa_samples	= Utility::GetMaxFramebufferSampleCount(*physical_device);

		this->logger->SimpleLog<decltype(this)>(
			Logging::LogLevel::Info,
			"Found a capable physical device: '%s'",
			physical_device->GetDeviceName().c_str()
		);
		this->logger->SimpleLog<decltype(this)>(
			Logging::LogLevel::Info,
			"Using MSAAx%u",
			physical_device->GetMSAASamples()
		);
	}

#pragma endregion
} // namespace Engine::Rendering::Vulkan
