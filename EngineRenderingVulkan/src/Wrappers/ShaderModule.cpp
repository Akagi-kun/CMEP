#include "Wrappers/ShaderModule.hpp"

#include "VulkanUtilities.hpp"
#include "Wrappers/Instance.hpp"
#include "Wrappers/InstanceOwned.hpp"
#include "Wrappers/LogicalDevice.hpp"

namespace Engine::Rendering::Vulkan
{
	ShaderModule::ShaderModule(
		InstanceOwned::value_t with_instance,
		std::filesystem::path with_path,
		const std::string& filename
	)
		: InstanceOwned(with_instance)
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		const auto shader_code = Vulkan::Utils::ReadShaderFile(with_path.append(filename));

		vk::ShaderModuleCreateInfo create_info(
			{},
			shader_code.size(),
			reinterpret_cast<const uint32_t*>(shader_code.data())
		);

		native_handle = logical_device->GetHandle().createShaderModule(create_info);
	}
} // namespace Engine::Rendering::Vulkan
