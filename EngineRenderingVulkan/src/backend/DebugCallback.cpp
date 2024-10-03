#include "Logging/Logging.hpp"

#include "Exception.hpp"

#define ENGINERENDERINGVULKAN_LIBRARY_IMPLEMENTATION
#include "backend/Instance.hpp"
#include "vulkan/vulkan.hpp"

namespace Engine::Rendering::Vulkan
{
	namespace
	{
		// NOLINTBEGIN(readability-identifier-naming)
		// this function has names as specified by Vulkan
		//
		VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT		messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT				messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void*										pUserData
		)
		{
			(void)(messageType);

			auto locked_logger =
				static_cast<Logging::SupportsLogging*>(pUserData)->getLogger();
			ENGINE_EXCEPTION_ON_ASSERT(
				locked_logger,
				"Failed locking logger on Lua print() call"
			)

			// Log as error only if error bit set
			Logging::LogLevel log_level =
				(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0
					? Logging::LogLevel::Error
					: Logging::LogLevel::Warning;

			locked_logger->simpleLog<void>(
				log_level,
				"Vulkan validation layer reported:\n%s",
				pCallbackData->pMessage
			);

			return vk::False;
		}
		// NOLINTEND(readability-identifier-naming)
	} // namespace

	PFN_vkDebugUtilsMessengerCallbackEXT debug_callback = vulkanDebugCallback;

} // namespace Engine::Rendering::Vulkan
