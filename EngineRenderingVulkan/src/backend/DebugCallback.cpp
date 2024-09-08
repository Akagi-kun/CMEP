#include "Logging/Logging.hpp"

#define ENGINERENDERINGVULKAN_LIBRARY_IMPLEMENTATION
#include "backend/Instance.hpp"
#include "vulkan/vulkan.hpp"

namespace Engine::Rendering::Vulkan
{
	// NOLINTBEGIN(readability-identifier-naming)
	// these functions do not follow our naming conventions
	// names are as specified by Vulkan
	//
	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT		messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT				messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void*										pUserData
	)
	{
		(void)(messageType);

		if (auto locked_logger = (static_cast<Logging::SupportsLogging*>(pUserData))->getLogger())
		{
			// Log as error only if error bit set
			Logging::LogLevel log_level = (messageSeverity &
										   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0
											  ? Logging::LogLevel::Error
											  : Logging::LogLevel::Warning;

			locked_logger->simpleLog<void>(
				log_level,
				"Vulkan validation layer reported:\n%s",
				pCallbackData->pMessage
			);
		}

		return vk::False;
	}

	PFN_vkDebugUtilsMessengerCallbackEXT debug_callback = VulkanDebugCallback;
	// NOLINTEND(readability-identifier-naming)

} // namespace Engine::Rendering::Vulkan
