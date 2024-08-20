#pragma once

#include "HoldsVMA.hpp"
#include "ImportVulkan.hpp"
#include "InstanceOwned.hpp"
#include "VulkanStructDefs.hpp"

#include <filesystem>

namespace Engine::Rendering::Vulkan
{
	class Pipeline : public InstanceOwned, public HoldsVMA
	{
	public:
		struct UserData
		{
			per_frame_array<Buffer*> uniform_buffers;

			vk::DescriptorPool descriptor_pool;
			per_frame_array<vk::DescriptorSet> descriptor_sets;

			[[nodiscard]] Buffer* GetUniformBuffer(uint32_t current_frame)
			{
				return uniform_buffers[current_frame];
			}

			[[nodiscard]] vk::DescriptorSet GetDescriptorSet(uint32_t current_frame)
			{
				return descriptor_sets[current_frame];
			}
		};

		Pipeline(
			InstanceOwned::value_t with_instance,
			RenderPass* with_render_pass,
			VulkanPipelineSettings settings,
			const std::filesystem::path& shader_path
		);
		~Pipeline();

		void AllocateNewUserData(UserData& into);

		void BindPipeline(UserData& from, vk::CommandBuffer with_command_buffer, uint32_t current_frame);

		static void UpdateDescriptorSets(
			vk::Device logical_device,
			UserData& from,
			per_frame_array<vk::WriteDescriptorSet> writes
		);

	private:
		vk::DescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
		VkPipelineLayout pipeline_layout			  = VK_NULL_HANDLE;
		vk::Pipeline native_handle					  = VK_NULL_HANDLE;

		std::vector<vk::DescriptorPoolSize> pool_sizes;

		void AllocateNewDescriptorPool(UserData& data_ref);
		void AllocateNewUniformBuffers(per_frame_array<Buffer*>& buffer_ref);
		void AllocateNewDescriptorSets(UserData& data_ref);
	};
} // namespace Engine::Rendering::Vulkan
