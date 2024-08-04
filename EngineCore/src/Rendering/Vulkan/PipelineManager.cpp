#include "Rendering/Vulkan/PipelineManager.hpp"

#include "Rendering/Vulkan/VDeviceManager.hpp"
#include "Rendering/Vulkan/Wrappers/HoldsVulkanDevice.hpp"
#include "Rendering/Vulkan/Wrappers/VPipeline.hpp"

namespace Engine::Rendering::Vulkan
{
	PipelineManager::PipelineManager(Engine* with_engine, VDeviceManager* with_device_manager)
		: InternalEngineObject(with_engine), HoldsVulkanDevice(with_device_manager)
	{
	}

	VPipeline* PipelineManager::GetPipeline(std::string ref)
	{
		return nullptr;
	}

} // namespace Engine::Rendering::Vulkan
