#include "PipelineManager.hpp"

#include "backend/Instance.hpp"
#include "objects/Buffer.hpp" // IWYU pragma: keep
#include "rendering/Pipeline.hpp"
#include "rendering/Swapchain.hpp"

#include <string_view>
#include <utility>

namespace Engine::Rendering::Vulkan
{
#pragma region Public

	PipelineManager::PipelineManager(
		SupportsLogging::logger_t with_logger,
		InstanceOwned::value_t with_instance,
		std::filesystem::path with_shader_path
	)
		: SupportsLogging(std::move(with_logger)), InstanceOwned(with_instance),
		  shader_path(std::move(with_shader_path))
	{
	}

	PipelineManager::~PipelineManager()
	{
		for (auto& [settings, pipeline_ptr] : pipelines)
		{
			delete pipeline_ptr;
		}

		pipelines.clear();
	}

	PipelineUserRef* PipelineManager::GetPipeline(const ExtendedPipelineSettings& with_settings)
	{
		Pipeline* pipeline = nullptr;
		std::string_view reason;

		std::tie(pipeline, reason) = this->FindPipeline(with_settings);

		if (pipeline != nullptr)
		{
			auto* user_ref = new PipelineUserRef(instance, pipeline);

			return user_ref;
		}

		this->logger->SimpleLog(
			Logging::LogLevel::Debug1,
			"Creating new pipeline (none found, '%s'), current pipelines: %u",
			reason.data(),
			this->pipelines.size()
		);

		// If no such pipeline is found, allocate new one
		pipeline = new Vulkan::Pipeline(
			instance,
			instance->GetWindow()->GetSwapchain()->GetRenderPass(),
			with_settings.short_setting,
			shader_path
		);

		this->pipelines.emplace_back(with_settings, pipeline);

		auto* user_ref = new PipelineUserRef(instance, pipeline);

		return user_ref;
	}

	PipelineUserRef::PipelineUserRef(InstanceOwned::value_t with_instance, Pipeline* with_origin)
		: InstanceOwned(with_instance), origin(with_origin)
	{
		user_data = origin->AllocateNewUserData();
	}

	PipelineUserRef::~PipelineUserRef()
	{
		for (auto* uniform_buffer : user_data->uniform_buffers)
		{
			delete uniform_buffer;
		}

		delete user_data;
	}

	void PipelineUserRef::UpdateDescriptorSets(per_frame_array<vk::WriteDescriptorSet> with_writes)
	{
		Pipeline::UpdateDescriptorSets(*instance->GetLogicalDevice(), *user_data, with_writes);
	}

	void PipelineUserRef::UpdateDescriptorSetsAll(const vk::WriteDescriptorSet& with_write)
	{
		per_frame_array<vk::WriteDescriptorSet> writes;
		std::fill(writes.begin(), writes.end(), with_write);

		UpdateDescriptorSets(writes);
	}

#pragma endregion

#pragma region Private

	// string_view is guaranteed to be null-terminated
	std::pair<Pipeline*, std::string_view> PipelineManager::FindPipeline(const ExtendedPipelineSettings& with_settings)
	{
		std::string_view reasons[] = {"no setting match", "setting match, no supply data match"};
		int reached_point		   = 0;

		// O(N)
		for (const auto& [settings, pipeline_ptr] : this->pipelines)
		{
			if (settings.short_setting == with_settings.short_setting)
			{
				if (settings.supply_data == with_settings.supply_data)
				{
					return {pipeline_ptr, {}};
				}

				reached_point = 1;
			}
		}

		return {nullptr, reasons[reached_point]};
	}

#pragma endregion
} // namespace Engine::Rendering::Vulkan
