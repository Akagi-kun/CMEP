#pragma once

#include "Rendering/Vulkan/Wrappers/HoldsVulkanDevice.hpp"

#include "vulkan/vulkan_core.h"

#include <filesystem>

namespace Engine::Rendering::Vulkan
{
	class ShaderModule final : public HoldsVulkanDevice
	{
	private:
		VkShaderModule native_handle;

	public:
		ShaderModule(DeviceManager* with_device_manager, std::filesystem::path with_path, const std::string& filename);
		~ShaderModule();

		[[nodiscard]] VkShaderModule GetNativeHandle()
		{
			return this->native_handle;
		}
	};
} // namespace Engine::Rendering::Vulkan
