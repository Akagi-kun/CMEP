#pragma once

#include "common/HandleWrapper.hpp"
#include "common/InstanceOwned.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <filesystem>


namespace Engine::Rendering::Vulkan
{
	class ShaderModule final : public HandleWrapper<vk::raii::ShaderModule>
	{
	public:
		// TODO: create with logical device
		ShaderModule(
			LogicalDevice* with_device,
			// InstanceOwned::value_t with_instance,
			std::filesystem::path with_path,
			const std::string& filename
		);
	};
} // namespace Engine::Rendering::Vulkan
