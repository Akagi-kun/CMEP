#pragma once
// IWYU pragma: private; include Rendering/Vulkan/rendering.hpp

#include "fwd.hpp"

#include "common/HandleWrapper.hpp"
#include "vulkan/vulkan_raii.hpp"

namespace Engine::Rendering::Vulkan
{
	class ShaderModule final : public HandleWrapper<vk::raii::ShaderModule>
	{
	public:
		const vk::ShaderStageFlagBits stage;

		ShaderModule(
			LogicalDevice*						with_device,
			vk::ShaderStageFlagBits				with_stage,
			const ShaderCompiler::spirv_code_t& with_code
		);

		vk::PipelineShaderStageCreateInfo getStageCreateInfo();
	};
} // namespace Engine::Rendering::Vulkan
