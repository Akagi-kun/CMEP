#pragma once

#include "ImportVulkan.hpp"
#include "common/HoldsVMA.hpp"
#include "common/InstanceOwned.hpp"
#include "common/StructDefs.hpp"
#include "rendering/PipelineSettings.hpp"

#include <filesystem>

namespace Engine::Rendering::Vulkan
{
	class Pipeline : public InstanceOwned, public HoldsVMA
	{
	public:
		struct UserData
		{
			per_frame_array<Buffer*> uniform_buffers;

			vk::raii::DescriptorPool descriptor_pool = nullptr;
			vk::raii::DescriptorSets descriptor_sets = nullptr;

			[[nodiscard]] Buffer* GetUniformBuffer(uint32_t current_frame)
			{
				return uniform_buffers[current_frame];
			}

			[[nodiscard]] vk::raii::DescriptorSet& GetDescriptorSet(uint32_t current_frame)
			{
				return descriptor_sets[current_frame];
			}
		};

		Pipeline(
			InstanceOwned::value_t with_instance,
			RenderPass* with_render_pass,
			PipelineSettings settings,
			const std::filesystem::path& shader_path
		);
		~Pipeline() = default;

		UserData* AllocateNewUserData();

		void BindPipeline(UserData& from, vk::CommandBuffer with_command_buffer, uint32_t current_frame);

		static void UpdateDescriptorSets(
			const vk::raii::Device& logical_device,
			UserData& from,
			per_frame_array<vk::WriteDescriptorSet> writes
		);

	private:
		vk::raii::DescriptorSetLayout descriptor_set_layout = nullptr;
		vk::raii::PipelineLayout pipeline_layout			= nullptr;
		vk::raii::Pipeline native_handle					= nullptr;

		std::vector<vk::DescriptorPoolSize> pool_sizes;

		// void AllocateNewUniformBuffers(per_frame_array<Buffer*>& buffer_ref);
		void AllocateNewDescriptorPool(UserData& data_ref);
		void AllocateNewDescriptorSets(UserData& data_ref);
	};
} // namespace Engine::Rendering::Vulkan
