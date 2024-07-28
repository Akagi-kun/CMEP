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
		const char* pipeline_name	= nullptr;
		Vulkan::VPipeline* pipeline = nullptr;

		RendererMatrixData matrix_data;

		IMeshBuilder* mesh_builder = nullptr;
		MeshBuildContext mesh_context{};

		// When false, UpdateMesh shall be internally called on next Render
		bool has_updated_mesh = false;

		// When false, UpdateMatrices will be called
		// Note that UpdateMatrices is also manually called from SceneManager
		bool has_updated_matrices = false;

	public:
		IRenderer(Engine* with_engine, IMeshBuilder* with_builder, const char* with_pipeline_program)
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
		}

		virtual void UpdateMesh()	  = 0;
		virtual void UpdateMatrices() = 0;

		void Render(VkCommandBuffer command_buffer, uint32_t current_frame)
		{
			if (!this->has_updated_matrices)
			{
				this->UpdateMatrices();
			}

			if (!this->has_updated_mesh)
			{
				this->UpdateMesh();
			}

			// If builder requests a rebuild, do it and update current context
			if (this->mesh_builder->NeedsRebuild())
			{
				this->mesh_builder->Build();
				this->mesh_context = this->mesh_builder->GetContext();
			}

			// Render only if VBO non-empty
			if (this->mesh_context.vbo_vert_count > 0)
			{
				this->pipeline->GetUniformBuffer(current_frame)
					->MemoryCopy(&this->matrix_data, sizeof(RendererMatrixData));

				this->pipeline->BindPipeline(command_buffer, current_frame);

				VkBuffer vertex_buffers[] = {this->mesh_context.vbo->GetNativeHandle()};
				VkDeviceSize offsets[]	  = {0};
				vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);

				vkCmdDraw(command_buffer, static_cast<uint32_t>(this->mesh_context.vbo_vert_count), 1, 0, 0);
			}
		}
	};
} // namespace Engine::Rendering
