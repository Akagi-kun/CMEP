#pragma once

#include "Rendering/IMeshBuilder.hpp"
#include "Rendering/Transform.hpp"
#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"

#include "InternalEngineObject.hpp"
#include "MeshBuildContext.hpp"
#include "SupplyData.hpp"
#include "Transform.hpp"
#include "Vulkan/VBuffer.hpp"
// #include "glm/glm.hpp"

#include <cstdint>

namespace Engine
{
	class Object;
	class Engine;
	class SceneManager;

	namespace Rendering
	{
		// Interface for Renderers
		class IRenderer : public InternalEngineObject
		{
		protected:
			Transform transform;
			Transform parent_transform;
			ScreenSize screen;

			VulkanPipeline* pipeline = nullptr;
			Vulkan::VBuffer* vbo	 = nullptr;
			size_t vbo_vert_count	 = 0;

			IMeshBuilder* mesh_builder = nullptr;
			MeshBuildContext mesh_context{};

			// If this is false, UpdateMesh shall be internally called on next Render
			bool has_updated_mesh = false;

		public:
			// std::weak_ptr<::Engine::SceneManager> scene_manager;

			IRenderer() = delete;
			IRenderer(Engine* engine, IMeshBuilder* with_builder)
				: InternalEngineObject(engine), mesh_builder(with_builder)
			{
			}
			virtual ~IRenderer()
			{
				// this->logger->SimpleLog(Logging::LogLevel::Debug2, "Cleaning up IRenderer");

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

				this->has_updated_mesh = false;
			}

			virtual void UpdateMesh() = 0;

			virtual void Render(VkCommandBuffer commandBuffer, uint32_t currentFrame) = 0;

			[[nodiscard]] virtual bool GetIsUI() const = 0;
		};
	} // namespace Rendering
} // namespace Engine
