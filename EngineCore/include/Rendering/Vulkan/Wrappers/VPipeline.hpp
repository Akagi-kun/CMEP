#pragma once

#include "Rendering/Vulkan/VulkanStructDefs.hpp"
#include "Rendering/Vulkan/Wrappers/HoldsVMA.hpp"
#include "Rendering/Vulkan/Wrappers/HoldsVulkanDevice.hpp"

#include "vulkan/vulkan_core.h"

namespace Engine::Rendering::Vulkan
{
	class VPipeline : public HoldsVulkanDevice, public HoldsVMA
	{
	private:
		VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
		VkPipelineLayout pipeline_layout			= VK_NULL_HANDLE;
		VkPipeline native_handle					= VK_NULL_HANDLE;

		std::vector<VBuffer*> uniform_buffers;

		VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> descriptor_sets;

	public:
		VPipeline(VDeviceManager* with_device_manager, VulkanPipelineSettings& settings, VkRenderPass with_render_pass);
		~VPipeline();

		[[nodiscard]] VBuffer* GetUniformBuffer(uint32_t current_frame)
		{
			return this->uniform_buffers.at(current_frame);
		}

		[[nodiscard]] VkDescriptorSet GetDescriptorSet(uint32_t current_frame)
		{
			return this->descriptor_sets.at(current_frame);
		}

		void BindPipeline(VkCommandBuffer with_command_buffer, uint32_t current_frame);
	};
} // namespace Engine::Rendering::Vulkan
