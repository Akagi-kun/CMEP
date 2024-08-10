#pragma once

#include "Rendering/SupplyData.hpp"
#include "Rendering/Vulkan/DeviceManager.hpp"
#include "Rendering/Vulkan/VulkanStructDefs.hpp"

#include "InternalEngineObject.hpp"
#include "Wrappers/HoldsVulkanDevice.hpp"
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

	class PipelineManager final : public InternalEngineObject, public HoldsVulkanDevice
	{
	private:
		std::vector<std::pair<ExtendedPipelineSettings, Pipeline*>> pipelines;

		Pipeline* FindPipeline(const ExtendedPipelineSettings& with_settings);

	public:
		PipelineManager(Engine* with_engine, DeviceManager* with_device_manager);
		~PipelineManager();

		std::tuple<size_t, Pipeline*> GetPipeline(const ExtendedPipelineSettings& with_settings);
	};
} // namespace Engine::Rendering::Vulkan
