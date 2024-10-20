#pragma once
// IWYU pragma: private; include Rendering/Vulkan/backend.hpp

#include "fwd.hpp"

#include "PhysicalDevice.hpp"
#include "PlatformSemantics.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <cassert>
#include <compare>
#include <string_view>
#include <vector>

namespace Engine::Rendering::Vulkan
{
	/**
	 * @property enable_vk_validation_layers
	 * @hideinitializer
	 */
#ifdef SEMANTICS_IS_DEBUG
	constexpr bool enable_vk_validation_layers = true;
	// NOLINTNEXTLINE(*unused-macros) This can be used to #ifdef
#	define DS_VALIDATION_LAYERS_ENABLED
#else
	constexpr bool enable_vk_validation_layers = false;
#endif

	struct DeviceScore final
	{
		PhysicalDevice device_scored;

		// Reason why the device is not supported, if any
		// Guaranteed to be null when supported and null-terminated if unsupported and a
		// reason is specified
		std::string_view unsupported_reason;

		int	 preference_score = 0;	   // Higher better
		bool supported		  = false; // Must be true

		DeviceScore(const PhysicalDevice& with_device, const Surface& with_surface);

		static const std::vector<const char*> device_extensions;
		static bool checkDeviceExtensionSupport(const vk::raii::PhysicalDevice& device);

		operator bool() const
		{
			return supported && (preference_score > 0);
		}

		/**
		 * Weak comparison operator, compares the score of this and some @p other device.
		 * @warning It is invalid to call this function when @ref operator bool() returns false.
		 */
		auto operator<=>(const DeviceScore& other) const
		{
			assert(
				static_cast<bool>(*this) && static_cast<bool>(other) &&
				"Tried to compare an unsupported device, possibly a bug?"
			);

			if (preference_score < other.preference_score)
			{
				return std::weak_ordering::less;
			}

			if (preference_score > other.preference_score)
			{
				return std::weak_ordering::greater;
			}

			return std::weak_ordering::equivalent;
		}
	};
} // namespace Engine::Rendering::Vulkan
