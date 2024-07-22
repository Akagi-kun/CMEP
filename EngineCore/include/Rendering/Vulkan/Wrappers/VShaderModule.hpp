#pragma once

#include "Rendering/Vulkan/HoldsVulkanDevice.hpp"

#include "vulkan/vulkan_core.h"

#include <filesystem>

namespace Engine::Rendering::Vulkan
{
	class VShaderModule final : public HoldsVulkanDevice
	{
	private:
		VkShaderModule native_handle;

	public:
		VShaderModule(VDeviceManager* with_device_manager, const std::filesystem::path& with_path);
		~VShaderModule();

		[[nodiscard]] VkShaderModule GetNativeHandle()
		{
			return this->native_handle;
		}
	};
} // namespace Engine::Rendering::Vulkan
