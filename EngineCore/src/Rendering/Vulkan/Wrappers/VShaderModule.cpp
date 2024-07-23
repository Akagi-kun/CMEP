#include "Rendering/Vulkan/Wrappers/VShaderModule.hpp"

#include "Rendering/Vulkan/VDeviceManager.hpp"
#include "Rendering/Vulkan/VulkanUtilities.hpp"

namespace Engine::Rendering::Vulkan
{
	VShaderModule::VShaderModule(
		VDeviceManager* with_device_manager,
		std::filesystem::path with_path,
		const std::string& filename
	)
		: HoldsVulkanDevice(with_device_manager)
	{
		VkDevice logical_device = this->device_manager->GetLogicalDevice();

		const auto shader_code = Vulkan::Utils::ReadShaderFile(with_path.append(filename));

		VkShaderModuleCreateInfo create_info{};
		create_info.sType	 = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.codeSize = shader_code.size();
		create_info.pCode	 = reinterpret_cast<const uint32_t*>(shader_code.data());

		if (vkCreateShaderModule(logical_device, &create_info, nullptr, &(this->native_handle)) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create shader module!");
		}
	}

	VShaderModule::~VShaderModule()
	{
		vkDestroyShaderModule(this->device_manager->GetLogicalDevice(), this->native_handle, nullptr);
	}
} // namespace Engine::Rendering::Vulkan
