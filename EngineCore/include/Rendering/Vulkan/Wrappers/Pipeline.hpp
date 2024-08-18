#pragma once

#include "Rendering/Vulkan/VulkanStructDefs.hpp"
#include "Rendering/Vulkan/Wrappers/HoldsVMA.hpp"
#include "Rendering/Vulkan/Wrappers/InstanceOwned.hpp"

#include "vulkan/vulkan_core.h"

#include <filesystem>

namespace Engine::Rendering::Vulkan
{
	class Pipeline : public InstanceOwned, public HoldsVMA
	{
	public:
		using user_idx_t = size_t;

		Pipeline(
			InstanceOwned::value_t with_instance,
			RenderPass* with_render_pass,
			VulkanPipelineSettings settings,
			const std::filesystem::path& shader_path
		);
		~Pipeline();

		user_idx_t AllocateNewUserData();

		void UpdateDescriptorSets(user_idx_t user_index, per_frame_array<VkWriteDescriptorSet> writes);
		void UpdateDescriptorSetsAll(user_idx_t user_index, const VkWriteDescriptorSet& with_write);

		[[nodiscard]] Buffer* GetUniformBuffer(user_idx_t user_idx, uint32_t current_frame)
		{
			return this->user_data[user_idx].uniform_buffers[current_frame];
		}

		[[nodiscard]] VkDescriptorSet GetDescriptorSet(user_idx_t user_idx, uint32_t current_frame)
		{
			return this->user_data[user_idx].descriptor_sets[current_frame];
		}

		void BindPipeline(user_idx_t user_index, VkCommandBuffer with_command_buffer, uint32_t current_frame);

	private:
		struct UserData
		{
			VkDescriptorPool with_pool;
			per_frame_array<Buffer*> uniform_buffers;
			per_frame_array<VkDescriptorSet> descriptor_sets;
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
		void AllocateNewUniformBuffers(per_frame_array<Buffer*>& buffer_ref);
		void AllocateNewDescriptorSets(UserData& data_ref);
	};
} // namespace Engine::Rendering::Vulkan
