#pragma once

#include "Rendering/SupplyData.hpp"

#include "Logging/Logging.hpp"

#include "VulkanStructDefs.hpp"
#include "Wrappers/InstanceOwned.hpp"
#include "Wrappers/Pipeline.hpp"

#include <utility>

namespace Engine::Rendering::Vulkan
{
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

		std::tuple<size_t, Pipeline*> GetPipeline(const ExtendedPipelineSettings& with_settings);

	private:
		std::vector<std::pair<ExtendedPipelineSettings, Pipeline*>> pipelines;

		Pipeline* FindPipeline(const ExtendedPipelineSettings& with_settings);

		std::filesystem::path shader_path;
	};
} // namespace Engine::Rendering::Vulkan
