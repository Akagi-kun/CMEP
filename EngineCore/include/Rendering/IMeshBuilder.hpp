#pragma once

#include "InternalEngineObject.hpp"
#include "MeshBuildContext.hpp"
#include "SupplyData.hpp"
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

			this->renderer->SyncDeviceWaitIdle();
			// vkDeviceWaitIdle(renderer->GetLogicalDevice());
			if (this->context.vbo != nullptr)
			{
				renderer->CleanupVulkanBuffer(this->context.vbo);
			}
		}

		virtual void SupplyData(const RendererSupplyData& data) = 0;

		virtual void Build() = 0;

		[[nodiscard]] const MeshBuildContext& GetContext() const
		{
			return this->context;
		}
	};
} // namespace Engine::Rendering
