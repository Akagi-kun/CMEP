#pragma once

#include "Rendering/SupplyData.hpp"

#include "Logging/Logging.hpp"

#include "VulkanStructDefs.hpp"
#include "Wrappers/InstanceOwned.hpp"
#include "Wrappers/Pipeline.hpp"

#include <utility>

namespace Engine::Rendering::Vulkan
{
	struct PipelineUserRef;

	struct ExtendedPipelineSettings
	{
		VulkanPipelineSettings short_setting;

		std::vector<RendererSupplyData> supply_data;

		ExtendedPipelineSettings(VulkanPipelineSettings from_setting) : short_setting(std::move(from_setting))
		{
		}
	};

	class PipelineManager final : public Logging::SupportsLogging, public InstanceOwned
	{
	public:
		PipelineManager(
			SupportsLogging::logger_t with_logger,
			InstanceOwned::value_t with_instance,
			std::filesystem::path with_shader_path
		);
		~PipelineManager();

		PipelineUserRef* GetPipeline(const ExtendedPipelineSettings& with_settings);

	private:
		std::filesystem::path shader_path;
		std::vector<std::pair<ExtendedPipelineSettings, Pipeline*>> pipelines;

		std::pair<Pipeline*, std::string_view> FindPipeline(const ExtendedPipelineSettings& with_settings);
	};

	struct PipelineUserRef final : public InstanceOwned
	{
	public:
		Pipeline::UserData* user_data = nullptr;

		PipelineUserRef(InstanceOwned::value_t with_instance, Pipeline* with_origin);
		~PipelineUserRef();

		// Disable copy
		PipelineUserRef(const PipelineUserRef&)			   = delete;
		PipelineUserRef& operator=(const PipelineUserRef&) = delete;

		operator bool() const
		{
			return origin != nullptr;
		}

		[[nodiscard]] auto GetUniformBuffer(uint32_t current_frame
		) const -> decltype(user_data->GetUniformBuffer(current_frame))
		{
			return user_data->GetUniformBuffer(current_frame);
		}

		void BindPipeline(vk::CommandBuffer with_command_buffer, uint32_t current_frame)
		{
			origin->BindPipeline(*user_data, with_command_buffer, current_frame);
		}

		void UpdateDescriptorSets(per_frame_array<vk::WriteDescriptorSet> with_writes);
		void UpdateDescriptorSetsAll(const vk::WriteDescriptorSet& with_write);

	private:
		Pipeline* origin = nullptr;
	};
} // namespace Engine::Rendering::Vulkan
