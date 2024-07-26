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
		std::vector<RenderingVertex> mesh;
		MeshBuildContext context = {};
		Vulkan::VulkanRenderingEngine* renderer;

	public:
		IMeshBuilder() = delete;
		IMeshBuilder(Engine* engine);

		virtual ~IMeshBuilder()
		{
			this->logger->SimpleLog(Logging::LogLevel::Debug3, "Cleaning up MeshBuilder");

			this->renderer->SyncDeviceWaitIdle();

			delete this->context.vbo;
		}

		virtual void SupplyData(const RendererSupplyData& data) = 0;

		virtual void Build() = 0;

		[[nodiscard]] virtual VkPrimitiveTopology GetSupportedTopology() const noexcept = 0;

		[[nodiscard]] const MeshBuildContext& GetContext() const
		{
			return this->context;
		}

		[[nodiscard]] bool HasRebuilt() noexcept
		{
			if (this->context.been_rebuilt)
			{
				this->context.been_rebuilt = false;
				return true;
			}

			return false;
		}
	};
} // namespace Engine::Rendering
