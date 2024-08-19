#pragma once

#include "HandleWrapper.hpp"
#include "ImportVulkan.hpp"
#include "InstanceOwned.hpp"

#include <filesystem>

namespace Engine::Rendering::Vulkan
{
	class ShaderModule final : public InstanceOwned, public HandleWrapper<vk::ShaderModule>
	{
	public:
		ShaderModule(
			InstanceOwned::value_t with_instance,
			std::filesystem::path with_path,
			const std::string& filename
		);
		~ShaderModule();
	};
} // namespace Engine::Rendering::Vulkan
