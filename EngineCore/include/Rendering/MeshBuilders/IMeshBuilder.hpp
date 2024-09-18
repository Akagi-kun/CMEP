#pragma once

#include "Rendering/MeshBuilders/MeshBuildContext.hpp"
#include "Rendering/SupplyData.hpp"

#include "InternalEngineObject.hpp"

#include <vector>

namespace Engine::Rendering
{
	class IMeshBuilder : public InternalEngineObject
	{
	public:
		IMeshBuilder() = delete;
		IMeshBuilder(Engine* engine);

		virtual ~IMeshBuilder()
		{
			delete context.vbo;
		}

		// Always call IMeshBuilder::SupplyData when overriding!
		virtual void supplyData(const MeshBuilderSupplyData& data)
		{
			needs_rebuild = true;
			(void)(data);
		}

		/**
		 * @todo Remove
		 */
		void supplyWorldPosition(const glm::vec3& with_world_position)
		{
			world_pos = with_world_position;
		}

		virtual void build() = 0;

		[[nodiscard]] virtual vk::PrimitiveTopology
		getSupportedTopology() const noexcept = 0;

		[[nodiscard]] const MeshBuildContext& getContext() const
		{
			return context;
		}

		[[nodiscard]] bool needsRebuild() const noexcept
		{
			return needs_rebuild;
		}

	protected:
		std::vector<RenderingVertex> mesh;
		bool						 needs_rebuild = true;

		MeshBuildContext  context = {};
		Vulkan::Instance* instance;

		glm::vec3 world_pos;
	};
} // namespace Engine::Rendering
