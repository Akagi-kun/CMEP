#pragma once
// IWYU pragma: private; include Rendering/Vulkan/rendering.hpp

#include "fwd.hpp"

#include "common/HandleWrapper.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <filesystem>
#include <string>

namespace Engine::Rendering::Vulkan
{
	class ShaderModule final : public HandleWrapper<vk::raii::ShaderModule>
	{
	public:
		ShaderModule(
			LogicalDevice*		  with_device,
			std::filesystem::path with_path,
			const std::string&	  filename
		);
	};
} // namespace Engine::Rendering::Vulkan
