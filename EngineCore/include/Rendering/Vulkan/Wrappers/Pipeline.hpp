#pragma once

#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"
#include "Rendering/Vulkan/VulkanStructDefs.hpp"
#include "Rendering/Vulkan/Wrappers/HoldsVMA.hpp"

#include "vulkan/vulkan_core.h"

#include <filesystem>

namespace Engine::Rendering::Vulkan
{
	class Pipeline : public InstanceOwned, public HoldsVMA
	{
	private:
		struct UserData
		{
			VkDescriptorPool with_pool;
			VulkanRenderingEngine::per_frame_array<Buffer*> uniform_buffers;
			VulkanRenderingEngine::per_frame_array<VkDescriptorSet> descriptor_sets;
		};

		VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
		VkPipelineLayout pipeline_layout			= VK_NULL_HANDLE;
		VkPipeline native_handle					= VK_NULL_HANDLE;

		// std::vector<VBuffer*> uniform_buffers;
		// std::vector<VulkanRenderingEngine::per_frame_array<VBuffer*>> uniform_buffers;

		std::vector<VkDescriptorPoolSize> pool_sizes;

		std::vector<UserData> user_data;

		// VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
		//  std::vector<VulkanRenderingEngine::per_frame_array<VkDescriptorSet>> descriptor_sets;

		void AllocateNewDescriptorPool(UserData& data_ref);
		void AllocateNewUniformBuffers(VulkanRenderingEngine::per_frame_array<Buffer*>& buffer_ref);
		void AllocateNewDescriptorSets(UserData& data_ref);

	public:
		Pipeline(
			InstanceOwned::value_t with_instance,
			RenderPass* with_render_pass,
			VulkanPipelineSettings settings,
			const std::filesystem::path& shader_path
		);
		~Pipeline();

		size_t AllocateNewUserData();

		void UpdateDescriptorSets(
			size_t user_index,
			const VulkanRenderingEngine::per_frame_array<VkWriteDescriptorSet>& writes
		);

		[[nodiscard]] Buffer* GetUniformBuffer(size_t user_idx, uint32_t current_frame)
		{
			return this->user_data[user_idx].uniform_buffers[current_frame];
		}

		[[nodiscard]] VkDescriptorSet GetDescriptorSet(size_t user_idx, uint32_t current_frame)
		{
			return this->user_data[user_idx].descriptor_sets[current_frame];
		}

		void BindPipeline(size_t user_index, VkCommandBuffer with_command_buffer, uint32_t current_frame);
	};
} // namespace Engine::Rendering::Vulkan
