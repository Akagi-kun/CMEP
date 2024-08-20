#include "Wrappers/Instance.hpp"

#include "Logging/Logging.hpp"

#include "GLFW/glfw3.h"
#include "Logging.hpp"
#include "Wrappers/CommandPool.hpp"
#include "Wrappers/DeviceScore.hpp"
#include "Wrappers/MemoryAllocator.hpp"
#include "Wrappers/Surface.hpp"

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
	// names are as specified by Vulkan
	//
	VKAPI_ATTR static VkBool32 VKAPI_CALL VulkanDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	)
	{
		(void)(messageType);

		if (auto locked_logger = (static_cast<Logging::SupportsLogging*>(pUserData))->GetLogger())
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

		// Initialize dynamic dispatcher base before all other vulkan calls
		VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

		InitInstance();

		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Instance initialized");

		window =
			new Window(this, with_window_parameters.size, with_window_parameters.title, with_window_parameters.hints);

		InitDevice();
		logical_device = new LogicalDevice(this, window->GetSurface());

		memory_allocator = new MemoryAllocator(this, *logical_device);

		command_pool = new CommandPool(this);

		window->CreateSwapchain();
	}

	Instance::~Instance()
	{
		logical_device->GetHandle().waitIdle();

		delete window;

		delete command_pool;

		delete memory_allocator;

		delete logical_device;

		if (enable_vk_validation_layers)
		{
			native_handle.destroyDebugUtilsMessengerEXT(debug_messenger);
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
		// Application information
		vk::ApplicationInfo app_info("A CMEP application", 1, "CMEP", 1, vk::ApiVersion11);

		// Check validation layer support
		if (enable_vk_validation_layers && !CheckVulkanValidationLayers())
		{
			throw std::runtime_error("Validation layers requested but unsupported!");
		}

		// Get extensions required by GLFW
		uint32_t glfw_extension_count = 0;
		const char** glfw_extensions  = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

		// Get our required extensions
		std::vector<const char*> instance_extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
		std::vector<const char*> instance_layers{};

		// Enable validation layer extension if it's a debug build
		if (enable_vk_validation_layers)
		{
			instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

			// Append validation layers if they're enabled
			instance_layers.insert(instance_layers.end(), validation_layers.begin(), validation_layers.end());
		}

		// Create an instance
		native_handle = vk::createInstance({
			{},
			&app_info,
			validation_layers,
			instance_extensions,
		});

		// Load instance functions in dispatcher
		VULKAN_HPP_DEFAULT_DISPATCHER.init(native_handle);

		this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Created a Vulkan instance");

		// If it's a debug build, add a debug callback to Vulkan
		if (enable_vk_validation_layers)
		{
			this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Creating debug messenger");

			vk::DebugUtilsMessengerCreateInfoEXT debug_messenger_create_info(
				{},
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
					vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
					vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
				vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
					vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
				VulkanDebugCallback,
				this
			);

			debug_messenger = native_handle.createDebugUtilsMessengerEXT(debug_messenger_create_info);

			this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Created debug messenger");
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
		this->logger->SimpleLog(Logging::LogLevel::Debug2, LOGPFX_CURRENT "Initializing vulkan device");

		// Get physical devices
		std::vector<vk::PhysicalDevice> physical_devices = native_handle.enumeratePhysicalDevices();

		// Sorted (by score) vector of all devices
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
				physical_device = candidate.device_scored;
				msaa_samples	= GetMaxUsableSampleCount(physical_device);

				this->logger->SimpleLog(
					Logging::LogLevel::Info,
					LOGPFX_CURRENT "Found a capable physical device: '%s'",
					physical_device.GetDeviceName().c_str()
				);
				this->logger->SimpleLog(Logging::LogLevel::Info, LOGPFX_CURRENT "Using MSAAx%u", msaa_samples);

				return;
			}
		}

		throw std::runtime_error("No physical device found!");
	}

	vk::SampleCountFlagBits Instance::GetMaxUsableSampleCount(vk::PhysicalDevice device)
	{
		vk::PhysicalDeviceProperties physical_device_properties = device.getProperties();

		// Check which sample counts are supported by the framebuffers
		vk::SampleCountFlags counts = physical_device_properties.limits.framebufferColorSampleCounts &
									  physical_device_properties.limits.framebufferDepthSampleCounts;

		if (!counts)
		{
			return vk::SampleCountFlagBits::e1;
		}

		if (counts & vk::SampleCountFlagBits::e64)
		{
			return vk::SampleCountFlagBits::e64;
		}
		if (counts & vk::SampleCountFlagBits::e32)
		{
			return vk::SampleCountFlagBits::e32;
		}
		if (counts & vk::SampleCountFlagBits::e16)
		{
			return vk::SampleCountFlagBits::e16;
		}
		if (counts & vk::SampleCountFlagBits::e8)
		{
			return vk::SampleCountFlagBits::e8;
		}
		if (counts & vk::SampleCountFlagBits::e4)
		{
			return vk::SampleCountFlagBits::e4;
		}
		if (counts & vk::SampleCountFlagBits::e2)
		{
			return vk::SampleCountFlagBits::e2;
		}

		return vk::SampleCountFlagBits::e1;
	}

#pragma endregion
} // namespace Engine::Rendering::Vulkan
