#include "PipelineManager.hpp"

#include "Logging/Logging.hpp"

#include "Exception.hpp"
#include "backend/Instance.hpp"
#include "objects/Buffer.hpp"
#include "rendering/Pipeline.hpp"
#include "rendering/Swapchain.hpp"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <memory>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace Engine::Rendering::Vulkan
{
#pragma region Public

	PipelineManager::PipelineManager(
		SupportsLogging::logger_t with_logger,
		InstanceOwned::value_t	  with_instance,
		std::filesystem::path	  with_shader_path
	)
		: SupportsLogging(std::move(with_logger)), InstanceOwned(with_instance),
		  shader_path(std::move(with_shader_path))
	{}

	PipelineManager::~PipelineManager()
	{
		this->logger->simpleLog<decltype(this)>(
			Logging::LogLevel::Debug,
			"Destructor called with %u pipelines left",
			pipelines.size()
		);

		pipelines.clear();
	}

	PipelineUserRef* PipelineManager::getPipeline(const PipelineSettings& with_settings)
	{
		std::shared_ptr<Pipeline> pipeline = nullptr;
		std::string_view		  reason;

		std::tie(pipeline, reason) = findPipeline(with_settings);

		if (pipeline != nullptr)
		{
			auto* user_ref = new PipelineUserRef(instance, pipeline);

			return user_ref;
		}

		this->logger->simpleLog<decltype(this)>(
			Logging::LogLevel::VerboseDebug,
			"Creating new pipeline (none found, '%s'), current pipelines: %u",
			reason.data(),
			pipelines.size()
		);

		// If no such pipeline is found, allocate new one
		pipeline = std::shared_ptr<Pipeline>(
			new Pipeline(
				instance,
				instance->getWindow()->getSwapchain()->getRenderPass(),
				with_settings,
				shader_path
			),
			// Pass a lambda deleter to remove it from the vector too
			[&](Pipeline* ptr) {
				this->pipelineDeallocCallback();
				delete ptr;
			}
		);

		pipelines.emplace_back(with_settings, pipeline);

		auto* user_ref = new PipelineUserRef(instance, pipeline);

		return user_ref;
	}

	PipelineUserRef::PipelineUserRef(
		InstanceOwned::value_t	  with_instance,
		std::shared_ptr<Pipeline> with_origin
	)
		: InstanceOwned(with_instance), origin(std::move(with_origin))
	{
		user_data = origin->allocateNewUserData();
	}

	PipelineUserRef::~PipelineUserRef()
	{
		for (auto* uniform_buffer : user_data->uniform_buffers)
		{
			delete uniform_buffer;
		}

		delete user_data;
	}

	void
	PipelineUserRef::updateDescriptorSets(per_frame_array<vk::WriteDescriptorSet> with_writes
	)
	{
		Pipeline::updateDescriptorSets(
			*instance->getLogicalDevice(),
			*user_data,
			with_writes
		);
	}

	void PipelineUserRef::updateDescriptorSetsAll(const vk::WriteDescriptorSet& with_write)
	{
		per_frame_array<vk::WriteDescriptorSet> writes;
		std::fill(writes.begin(), writes.end(), with_write);

		updateDescriptorSets(writes);
	}

#pragma endregion

#pragma region Private

	// string_view is guaranteed to be null-terminated
	std::pair<std::shared_ptr<Pipeline>, std::string_view>
	PipelineManager::findPipeline(const PipelineSettings& with_settings)
	{
		std::string_view reasons[]	   = {"no setting match"};
		int				 reached_point = 0;

		// O(N)
		for (const auto& [settings, pipeline_ptr] : pipelines)
		{
			if (settings == with_settings)
			{
				auto locked_pipeline = pipeline_ptr.lock();

				ENGINE_EXCEPTION_ON_ASSERT(locked_pipeline, "Failed locking pipeline")

				return {locked_pipeline, {}};
			}
		}

		return {nullptr, reasons[reached_point]};
	}

	// Called when a pipeline's ref counter reaches 0
	void PipelineManager::pipelineDeallocCallback()
	{
		// Delete all entries that are expired
		std::erase_if(pipelines, [](auto pred_val) {
			return std::get<1>(pred_val).expired();
		});
	}

#pragma endregion
} // namespace Engine::Rendering::Vulkan
