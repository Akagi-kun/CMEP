#include "Wrappers/ShaderModule.hpp"

#include "VulkanStructDefs.hpp"
#include "VulkanUtilities.hpp"
#include "Wrappers/Instance.hpp"
#include "Wrappers/InstanceOwned.hpp"
#include "Wrappers/LogicalDevice.hpp"
#include "vulkan/vulkan_structs.hpp"

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

		/* VkShaderModuleCreateInfo create_info{};
		create_info.sType	 = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.codeSize = shader_code.size();
		create_info.pCode	 = reinterpret_cast<const uint32_t*>(shader_code.data()); */

		vk::ShaderModuleCreateInfo create_info(
			{},
			shader_code.size(),
			reinterpret_cast<const uint32_t*>(shader_code.data())
		);

		native_handle = logical_device->GetHandle().createShaderModule(create_info);
		/* if (vkCreateShaderModule(logical_device->GetHandle(), &create_info, nullptr, &(this->native_handle)) !=
			  VK_SUCCESS)
		{
			throw std::runtime_error("failed to create shader module!");
		} */
	}

	ShaderModule::~ShaderModule()
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		logical_device->GetHandle().destroyShaderModule(native_handle);
		// vkDestroyShaderModule(logical_device->GetHandle(), this->native_handle, nullptr);
	}
} // namespace Engine::Rendering::Vulkan
