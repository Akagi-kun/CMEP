#pragma once
// IWYU pragma: private; include Rendering/Vulkan/rendering.hpp

#include "fwd.hpp"

#include "common/HandleWrapper.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <filesystem>

namespace Engine::Rendering::Vulkan
{
	class ShaderModule final : public HandleWrapper<vk::raii::ShaderModule>
	{
	public:
		ShaderModule(LogicalDevice* with_device, const std::filesystem::path& filepath);
	};
} // namespace Engine::Rendering::Vulkan
