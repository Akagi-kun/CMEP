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
		// bool been_rebuilt  = false;
		bool needs_rebuild = true;

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

		// Always call IMeshBuilder::SupplyData when overriding!
		virtual void SupplyData(const RendererSupplyData& data)
		{
			this->needs_rebuild = true;
			(void)(data);
		}

		virtual void Build() = 0;

		[[nodiscard]] virtual VkPrimitiveTopology GetSupportedTopology() const noexcept = 0;

		[[nodiscard]] const MeshBuildContext& GetContext() const
		{
			return this->context;
		}

		/* [[nodiscard]] bool HasRebuilt() noexcept
		{
			if (this->been_rebuilt)
			{
				this->been_rebuilt = false;
				return true;
			}

			return false;
		} */

		[[nodiscard]] bool NeedsRebuild() const noexcept
		{
			return this->needs_rebuild;
		}
	};
} // namespace Engine::Rendering
