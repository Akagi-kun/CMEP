#pragma once

#include "Rendering/Vulkan/VDeviceManager.hpp"

#include "InternalEngineObject.hpp"
#include "Wrappers/HoldsVulkanDevice.hpp"
#include "Wrappers/VPipeline.hpp"

#include <unordered_map>

namespace Engine::Rendering::Vulkan
{
	class PipelineManager final : public InternalEngineObject, public HoldsVulkanDevice
	{
	private:
		std::unordered_map<std::string, VPipeline*> pipelines;

	public:
		PipelineManager(Engine* with_engine, VDeviceManager* with_device_manager);
		~PipelineManager() = default;

		VPipeline* GetPipeline(std::string ref);
	};
} // namespace Engine::Rendering::Vulkan
