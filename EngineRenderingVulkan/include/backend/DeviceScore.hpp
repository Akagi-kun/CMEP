#pragma once
// IWYU pragma: private; include Rendering/Vulkan/backend.hpp

#include "fwd.hpp"

#include "PhysicalDevice.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <cassert>
#include <functional>
#include <string_view>
#include <vector>

namespace Engine::Rendering::Vulkan
{
#ifndef _DEBUG
	static constexpr bool enable_vk_validation_layers = false;
#else
	static constexpr bool enable_vk_validation_layers = true;
#	define DS_VALIDATION_LAYERS_ENABLED // NOLINT this can be used to #ifdef
#endif

	struct DeviceScore final
	{
		PhysicalDevice device_scored;

		// Reason why the device is not supported, if any
		// Guaranteed to be null when supported and zero-terminated if unsupported and a reason is
		// specified
		std::string_view unsupported_reason;

		int	 preference_score = 0;	   // Higher better
		bool supported		  = false; // Must be true

		DeviceScore(const PhysicalDevice& with_device, const Surface* with_surface);

		static const std::vector<const char*> device_extensions;
		static bool checkDeviceExtensionSupport(const vk::raii::PhysicalDevice& device);

		operator bool() const
		{
			return supported && (preference_score > 0);
		}

		bool operator<(const DeviceScore& other) const
		{
			assert(
				supported && "Tried to call operator< on an unsupported device, possibly a bug?"
			);

			return std::less<>{}(preference_score, other.preference_score);
		}
	};
} // namespace Engine::Rendering::Vulkan
