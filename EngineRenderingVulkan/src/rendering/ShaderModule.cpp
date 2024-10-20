#include "rendering/ShaderModule.hpp"

#include "backend/LogicalDevice.hpp"
#include "common/Utility.hpp"

#include <cassert>

namespace Engine::Rendering::Vulkan
{
	ShaderModule::ShaderModule(
		LogicalDevice*						with_device,
		vk::ShaderStageFlagBits				with_stage,
		const ShaderCompiler::spirv_code_t& with_code
	)
		: stage(with_stage)
	{
		vk::ShaderModuleCreateInfo create_info{
			.codeSize = with_code.size() * sizeof(ShaderCompiler::spirv_code_t::value_type),
			.pCode = with_code.data()
		};

		native_handle = with_device->createShaderModule(create_info);
	}

	vk::PipelineShaderStageCreateInfo ShaderModule::getStageCreateInfo()
	{
		return vk::PipelineShaderStageCreateInfo{
			.stage	= stage,
			.module = native_handle,
			.pName	= "main"
		};
	}
} // namespace Engine::Rendering::Vulkan
