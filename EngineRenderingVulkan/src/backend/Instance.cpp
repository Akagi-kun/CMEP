#include "Exception.hpp"

#include <functional>
#include <type_traits>

#define ENGINERENDERINGVULKAN_LIBRARY_IMPLEMENTATION
#include "Logging/Logging.hpp"

#include "backend/DeviceScore.hpp"
#include "backend/Instance.hpp"
#include "backend/LogicalDevice.hpp"
#include "backend/MemoryAllocator.hpp"
#include "backend/PhysicalDevice.hpp"
#include "objects/CommandPool.hpp"
#include "rendering/Surface.hpp"
#include "rendering/Window.hpp"
#include "vulkan/vulkan.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <format>
#include <string>
#include <utility>
#include <vector>

namespace Engine::Rendering::Vulkan
{
#pragma region Internal static

	namespace
	{
		[[noreturn]] void glfwErrorCallback(int error, const char* description)
		{
			throw ENGINE_EXCEPTION(std::format(
				"GLFW error handler callback called! Code: '{}'; description: "
				"'{}'",
				error,
				description
			));
		}
	} // namespace

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
			throw ENGINE_EXCEPTION("GLFW returned GLFW_FALSE on glfwInit!");
		}
		glfwSetErrorCallback(glfwErrorCallback);

		this->logger->logSingle<decltype(this)>(Logging::LogLevel::Info, "GLFW initialized");

		// Initialize dynamic dispatcher base before all other vulkan calls
		VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

		initInstance();

		this->logger->logSingle<decltype(this)>(Logging::LogLevel::Info, "Instance initialized");

		window = new Window(
			this,
			with_window_parameters.size,
			with_window_parameters.title,
			with_window_parameters.hints
		);

		initDevice();
		logical_device = new LogicalDevice(this, window->getSurface());

		memory_allocator = new MemoryAllocator(this, *logical_device);

		command_pool = new CommandPool(this);

		window->createSwapchain();
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

	const std::vector<const char*> validation_layers = {
		"VK_LAYER_KHRONOS_validation",
	};

	void Instance::initInstance()
	{
		// Application information
		vk::ApplicationInfo app_info{
			.pApplicationName	= "A CMEP application",
			.applicationVersion = 1,
			.pEngineName		= "CMEP",
			.engineVersion		= 1,
			.apiVersion			= vk::ApiVersion12
		};

		// Check validation layer support
		if (enable_vk_validation_layers && !checkVulkanValidationLayers())
		{
			throw ENGINE_EXCEPTION("Validation layers requested but unsupported!");
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
			instance_extensions.push_back(vk::EXTDebugUtilsExtensionName);
		}

		native_handle = context.createInstance({
			.pApplicationInfo		 = &app_info,
			.enabledLayerCount		 = static_cast<uint32_t>(instance_layers.size()),
			.ppEnabledLayerNames	 = instance_layers.data(),
			.enabledExtensionCount	 = static_cast<uint32_t>(instance_extensions.size()),
			.ppEnabledExtensionNames = instance_extensions.data(),
		});

		// Load instance functions in dispatcher
		VULKAN_HPP_DEFAULT_DISPATCHER.init(*native_handle);

		this->logger->logSingle<decltype(this)>(Logging::LogLevel::Debug, "Created instance object");

		// If it's a debug build, add a debug callback to Vulkan
		if (enable_vk_validation_layers)
		{
			this->logger->logSingle<decltype(this)>(
				Logging::LogLevel::VerboseDebug,
				"Creating debug messenger"
			);

			vk::DebugUtilsMessengerCreateInfoEXT messenger_create_info{
				.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
								   vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
								   vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
				.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
							   vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
							   vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
				.pfnUserCallback = debug_callback,
				.pUserData		 = this
			};

			debug_messenger = native_handle.createDebugUtilsMessengerEXT(messenger_create_info);

			this->logger->logSingle<decltype(this)>(
				Logging::LogLevel::VerboseDebug,
				"Created debug messenger"
			);
		}
	}

	bool Instance::checkVulkanValidationLayers()
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

	void Instance::initDevice()
	{
		this->logger->logSingle<decltype(this)>(Logging::LogLevel::Debug, "Initializing device");

		// Get all physical devices
		std::vector<vk::raii::PhysicalDevice> physical_devices =
			native_handle.enumeratePhysicalDevices();

		// Sorted (by score) vector of all devices
		std::vector<DeviceScore> candidates;
		candidates.reserve(physical_devices.size());
		for (const auto& device : physical_devices)
		{
			// Check device suitability and generate score based on it's parameters
			auto score = DeviceScore(device, window->getSurface());

			this->logger->logSingle<decltype(this)>(
				Logging::LogLevel::VerboseDebug,
				"Found device '{}'; score {}; {}{}",
				score.device_scored.getDeviceName(),
				score.preference_score,
				score ? "suitable" : "unsuitable ",
				score ? "" : score.unsupported_reason
			);

			// Emplace only devices that are supported
			if (score)
			{
				candidates.emplace_back(score);
			}
		}

		if (candidates.empty())
		{
			throw ENGINE_EXCEPTION("No physical device found!");
		}

		// Sort devices, using std::greater to achieve the descending order
		std::sort(candidates.begin(), candidates.end(), std::greater<>());

		// First device is assumed to be the best after sort
		physical_device = new PhysicalDevice(candidates[0].device_scored);

		this->logger->logSingle<decltype(this)>(
			Logging::LogLevel::Info,
			"Found a capable physical device: '{}'",
			physical_device->getDeviceName()
		);
		this->logger->logSingle<decltype(this)>(
			Logging::LogLevel::Info,
			"Using MSAAx{}",
			static_cast<std::underlying_type_t<vk::SampleCountFlagBits>>(
				physical_device->getMSAASamples()
			)
		);
	}

#pragma endregion
} // namespace Engine::Rendering::Vulkan
