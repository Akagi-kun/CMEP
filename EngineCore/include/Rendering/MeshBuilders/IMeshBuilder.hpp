#pragma once

#include "Rendering/MeshBuilders/MeshBuildContext.hpp"
#include "Rendering/SupplyData.hpp"

#include "InternalEngineObject.hpp"

namespace Engine::Rendering
{
	class IMeshBuilder : public InternalEngineObject
	{
	public:
		IMeshBuilder() = delete;
		IMeshBuilder(Engine* engine);

		virtual ~IMeshBuilder()
		{
			this->logger->SimpleLog(Logging::LogLevel::Debug3, "Cleaning up MeshBuilder");

			delete this->context.vbo;
		}

		// Always call IMeshBuilder::SupplyData when overriding!
		virtual void SupplyData(const RendererSupplyData& data)
		{
			this->needs_rebuild = true;
			(void)(data);
		}

		void SupplyWorldPosition(const glm::vec3& with_world_position)
		{
			this->world_pos = with_world_position;
		}

		virtual void Build() = 0;

		[[nodiscard]] virtual vk::PrimitiveTopology GetSupportedTopology() const noexcept = 0;

		[[nodiscard]] const MeshBuildContext& GetContext() const
		{
			return this->context;
		}

		[[nodiscard]] bool NeedsRebuild() const noexcept
		{
			return this->needs_rebuild;
		}

	protected:
		std::vector<RenderingVertex> mesh;
		bool needs_rebuild = true;

		MeshBuildContext context = {};
		Vulkan::Instance* instance;

		glm::vec3 world_pos;
	};
} // namespace Engine::Rendering
