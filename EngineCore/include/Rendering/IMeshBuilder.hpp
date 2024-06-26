#pragma once

#include "InternalEngineObject.hpp"
#include "MeshBuildContext.hpp"
#include "Vulkan/VulkanRenderingEngine.hpp"

namespace Engine::Rendering
{
	class IMeshBuilder : public InternalEngineObject
	{
	protected:
		MeshBuildContext context = {};
		VulkanRenderingEngine* renderer;

	public:
		IMeshBuilder() = delete;
		IMeshBuilder(Engine* engine, VulkanRenderingEngine* with_renderer)
			: InternalEngineObject(engine), renderer(with_renderer)
		{
		}
		virtual ~IMeshBuilder()
		{
			this->logger->SimpleLog(Logging::LogLevel::Debug3, "Cleaning up MeshBuilder");

			vkDeviceWaitIdle(renderer->GetLogicalDevice());
			if (this->context.vbo != nullptr)
			{
				renderer->CleanupVulkanBuffer(this->context.vbo);
			}
		}

		virtual void Build() = 0;

		const MeshBuildContext& GetContext()
		{
			return this->context;
		}
	};
} // namespace Engine::Rendering
