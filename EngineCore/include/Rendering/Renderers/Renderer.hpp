#pragma once

#include "Rendering/MeshBuilders/IMeshBuilder.hpp"
#include "Rendering/MeshBuilders/MeshBuildContext.hpp"
#include "Rendering/SupplyData.hpp"
#include "Rendering/Transform.hpp"
#include "Rendering/Vulkan/exports.hpp"

#include "InternalEngineObject.hpp"

#include <cstdint>

namespace Engine::Rendering
{
	// Interface for Renderers
	class IRenderer : public InternalEngineObject
	{
	public:
		IRenderer(Engine* with_engine, IMeshBuilder* with_builder, std::string_view with_pipeline_program);
		virtual ~IRenderer();

		// Renderers shall implement this to get textures, fonts etc.
		void SupplyData(const RendererSupplyData& data);

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

		void Render(Vulkan::CommandBuffer* command_buffer, uint32_t current_frame);

	protected:
		Transform transform;
		Transform parent_transform;
		ScreenSize screen;

		// Renderer configuration
		std::string_view pipeline_name;
		Vulkan::PipelineUserRef* pipeline;
		// Vulkan::Pipeline* pipeline = nullptr;
		// size_t pipeline_user_index = 0;
		std::shared_ptr<Vulkan::PipelineManager> pipeline_manager;

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

	private:
		Vulkan::PipelineSettings settings;
	};

	class Renderer3D final : public IRenderer
	{
	public:
		using IRenderer::IRenderer;

		void UpdateMatrices() override;
	};

	class Renderer2D final : public IRenderer
	{
	public:
		using IRenderer::IRenderer;

		void UpdateMatrices() override;
	};
} // namespace Engine::Rendering
