#include "rendering/ShaderModule.hpp"

#include "backend/LogicalDevice.hpp"
#include "common/Utility.hpp"

#include <cstdint>
#include <filesystem>

namespace Engine::Rendering::Vulkan
{
	ShaderModule::ShaderModule(
		LogicalDevice*				 with_device,
		const std::filesystem::path& filepath
	)
	{
		const auto shader_code = Vulkan::Utility::readShaderFile(filepath);

		vk::ShaderModuleCreateInfo create_info{
			.codeSize = shader_code.size(),
			.pCode	  = reinterpret_cast<const uint32_t*>(shader_code.data())
		};

		native_handle = with_device->createShaderModule(create_info);
	}
} // namespace Engine::Rendering::Vulkan
