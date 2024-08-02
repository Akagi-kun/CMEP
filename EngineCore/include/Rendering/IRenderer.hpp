#pragma once

#include "Rendering/IMeshBuilder.hpp"
#include "Rendering/Transform.hpp"
#include "Rendering/Vulkan/Wrappers/VPipeline.hpp"

#include "InternalEngineObject.hpp"
#include "MeshBuildContext.hpp"
#include "SupplyData.hpp"
#include "Transform.hpp"

#include <cstdint>

namespace Engine::Rendering
{
	struct RendererMatrixData
	{
		glm::mat4 mat_vp{};
		glm::mat4 mat_model{};
	};

	// Interface for Renderers
	class IRenderer : public InternalEngineObject
	{
	protected:
		Transform transform;
		Transform parent_transform;
		ScreenSize screen;

		// Renderer configuration
		std::string_view pipeline_name;
		Vulkan::VPipeline* pipeline = nullptr;

		IMeshBuilder* mesh_builder = nullptr;
		MeshBuildContext mesh_context{};

		std::shared_ptr<Rendering::Texture> texture = nullptr;
		RendererMatrixData matrix_data;

		// When false, UpdateDescriptorSets shall be internally called on next Render
		bool has_updated_descriptors = false;
		void UpdateDescriptorSets();

		// When false, UpdateMatrices will be called
		// Note that UpdateMatrices is also manually called from SceneManager
		bool has_updated_matrices = false;

	public:
		IRenderer(Engine* with_engine, IMeshBuilder* with_builder, std::string_view with_pipeline_program)
			: InternalEngineObject(with_engine), pipeline_name(with_pipeline_program), mesh_builder(with_builder)
		{
		}
		virtual ~IRenderer()
		{
			delete this->mesh_builder;
			this->mesh_builder = nullptr;
		}

		// Renderers shall implement this to get textures, fonts etc.
		virtual void SupplyData(const RendererSupplyData& data) = 0;

		// Renderers shall implement this to update their matrix_data
		virtual void UpdateMatrices() = 0;

		void UpdateTransform(
			const Transform& with_transform,
			const Transform& with_parent_transform,
			const ScreenSize& with_screen
		)
		{
			this->transform		   = with_transform;
			this->parent_transform = with_parent_transform;
			this->screen		   = with_screen;

			this->has_updated_matrices = false;

			this->mesh_builder->SupplyWorldPosition(with_transform.pos + with_parent_transform.pos);
		}

		void ForceBuild()
		{
			this->mesh_builder->Build();
		}

		void Render(Vulkan::VCommandBuffer* command_buffer, uint32_t current_frame);
	};
} // namespace Engine::Rendering
