#include "rendering/ShaderModule.hpp"

#include "backend/LogicalDevice.hpp"
#include "common/Utility.hpp"

#include <cstdint>
#include <filesystem>
#include <string>

namespace Engine::Rendering::Vulkan
{
	ShaderModule::ShaderModule(
		LogicalDevice*		  with_device,
		std::filesystem::path with_path,
		const std::string&	  filename
	)
	{
		const auto shader_code = Vulkan::Utility::readShaderFile(with_path.append(filename));

		vk::ShaderModuleCreateInfo create_info(
			{},
			shader_code.size(),
			reinterpret_cast<const uint32_t*>(shader_code.data())
		);

		native_handle = with_device->createShaderModule(create_info);
	}
} // namespace Engine::Rendering::Vulkan
