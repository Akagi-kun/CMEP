#pragma once
// IWYU pragma: private; include Rendering/Vulkan/rendering.hpp

#include "fwd.hpp"

#include "common/HoldsVMA.hpp"
#include "common/InstanceOwned.hpp"
#include "common/StructDefs.hpp"
#include "rendering/PipelineSettings.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <cstdint>
#include <filesystem>
#include <vector>

namespace Engine::Rendering::Vulkan
{
	class Pipeline : public InstanceOwned, public HoldsVMA
	{
	public:
		/**
		 * @brief Represents a set of data related to a singular user of this pipeline
		 *
		 * Using this method similar pipelines can be shared
		 *
		 * @note User does not denote the user of the application, but an internal class
		 *       that uses this struct to store its data
		 */
		struct UserData
		{
			per_frame_array<UniformBuffer*> uniform_buffers;

			vk::raii::DescriptorPool descriptor_pool = nullptr;
			vk::raii::DescriptorSets descriptor_sets = nullptr;

			[[nodiscard]] UniformBuffer* getUniformBuffer(uint32_t current_frame)
			{
				return uniform_buffers[current_frame];
			}

			[[nodiscard]] vk::raii::DescriptorSet& getDescriptorSet(uint32_t current_frame)
			{
				return descriptor_sets[current_frame];
			}
		};

		Pipeline(
			InstanceOwned::value_t		 with_instance,
			RenderPass*					 with_render_pass,
			PipelineSettings			 settings,
			const std::filesystem::path& shader_path
		);
		~Pipeline() = default;

		[[nodiscard]] UserData* allocateNewUserData();

		void bindPipeline(
			UserData&		  userdata_ref,
			vk::CommandBuffer with_command_buffer,
			uint32_t		  current_frame
		);

		static void updateDescriptorSets(
			const vk::raii::Device&					logical_device,
			UserData&								from,
			per_frame_array<vk::WriteDescriptorSet> writes
		);

	private:
		vk::raii::DescriptorSetLayout descriptor_set_layout = nullptr;
		vk::raii::PipelineLayout	  pipeline_layout		= nullptr;
		vk::raii::Pipeline			  native_handle			= nullptr;

		std::vector<vk::DescriptorPoolSize> pool_sizes;

		void allocateNewDescriptorPool(UserData& data_ref);
		void allocateNewDescriptorSets(UserData& data_ref);
	};
} // namespace Engine::Rendering::Vulkan
