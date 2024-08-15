#include "Rendering/Vulkan/PipelineManager.hpp"

#include "Rendering/Vulkan/Wrappers/Pipeline.hpp"
#include "Rendering/Vulkan/Wrappers/Swapchain.hpp"

#include "Engine.hpp"

#include <utility>

namespace Engine::Rendering::Vulkan
{
	PipelineManager::PipelineManager(Engine* with_engine, InstanceOwned::value_t with_instance)
		: InternalEngineObject(with_engine), InstanceOwned(with_instance)
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

	Pipeline* PipelineManager::FindPipeline(const ExtendedPipelineSettings& with_settings)
	{
		// O(N)
		for (const auto& [settings, pipeline_ptr] : this->pipelines)
		{
			if (settings.short_setting == with_settings.short_setting)
			{
				if (settings.supply_data == with_settings.supply_data)
				{
					return pipeline_ptr;
				}

				return nullptr;
			}
		}

		return nullptr;
	}

	std::tuple<size_t, Pipeline*> PipelineManager::GetPipeline(const ExtendedPipelineSettings& with_settings)
	{
		Pipeline* pipeline = /*  nullptr;  */ this->FindPipeline(with_settings);

		if (pipeline != nullptr)
		{
			size_t new_user_index = pipeline->AllocateNewUserData();
			return {new_user_index, pipeline};
		}

		this->logger->SimpleLog(
			Logging::LogLevel::Debug1,
			"Creating new pipeline (no usable pipeline found), current pipelines: %u",
			this->pipelines.size()
		);

		// If no such pipeline is found, allocate new one
		pipeline = new Vulkan::Pipeline(
			instance,
			instance->GetWindow()->GetSwapchain()->GetRenderPass(), // renderer->GetSwapchain()->GetRenderPass(),
			with_settings.short_setting,
			this->owner_engine->GetShaderPath()
		);

		this->pipelines.emplace_back(with_settings, pipeline);

		size_t new_user_index = pipeline->AllocateNewUserData();
		return {new_user_index, pipeline};
	}

} // namespace Engine::Rendering::Vulkan
