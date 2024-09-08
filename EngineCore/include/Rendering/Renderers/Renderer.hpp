#pragma once

#include "Rendering/MeshBuilders/IMeshBuilder.hpp"
#include "Rendering/MeshBuilders/MeshBuildContext.hpp"
#include "Rendering/SupplyData.hpp"
#include "Rendering/Transform.hpp"
#include "Rendering/Vulkan/exports.hpp"

#include "InternalEngineObject.hpp"

#include <cstdint>
#include <memory>
#include <string_view>

namespace Engine::Rendering
{
	// Interface for Renderers
	class IRenderer : public InternalEngineObject
	{
	public:
		IRenderer(
			Engine*			 with_engine,
			IMeshBuilder*	 with_builder, // TODO: Make const
			std::string_view with_pipeline_program
		);
		virtual ~IRenderer();

		void supplyData(const RendererSupplyData& data);

		// Renderers shall implement this to update their matrix_data
		virtual void updateMatrices() = 0;

		void updateTransform(
			const Transform&  with_transform,
			const Transform&  with_parent_transform,
			const ScreenSize& with_screen
		)
		{
			transform		 = with_transform;
			parent_transform = with_parent_transform;
			screen			 = with_screen;

			has_updated_matrices = false;

			// TODO: Remove
			mesh_builder->supplyWorldPosition(with_transform.pos + with_parent_transform.pos);
		}

		void render(Vulkan::CommandBuffer* command_buffer, uint32_t current_frame);

	protected:
		Transform  transform;
		Transform  parent_transform;
		ScreenSize screen;

		// Renderer configuration
		std::string_view pipeline_name;

		Vulkan::PipelineUserRef*				 pipeline = nullptr;
		std::shared_ptr<Vulkan::PipelineManager> pipeline_manager;

		IMeshBuilder*	 mesh_builder = nullptr;
		MeshBuildContext mesh_context{};

		std::shared_ptr<Rendering::Texture> texture = nullptr;
		RendererMatrixData					matrix_data;

		// When false, UpdateDescriptorSets shall be internally called on next Render
		bool has_updated_descriptors = false;
		void updateDescriptorSets();

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

		void updateMatrices() override;
	};

	class Renderer2D final : public IRenderer
	{
	public:
		using IRenderer::IRenderer;

		void updateMatrices() override;
	};
} // namespace Engine::Rendering
