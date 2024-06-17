#pragma once

#include "Rendering/Transform.hpp"
#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"

#include "IModule.hpp"
#include "InternalEngineObject.hpp"
#include "SupplyData.hpp"
#include "Transform.hpp"
// #include "glm/glm.hpp"

#include <cstdint>

namespace Engine
{
	class Object;
	class Engine;
	class SceneManager;

	namespace Rendering
	{
		class Shader;

		// Interface for Renderers
		class IRenderer : public IModule
		{
		protected:
			Transform transform;
			Transform parent_transform;

			ScreenSize screen;

			// If this is false, UpdateMesh shall be internally called on next Render
			bool has_updated_mesh = false;

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

			virtual void SupplyData(const RendererSupplyData& data) = 0;

		public:
			std::weak_ptr<::Engine::SceneManager> scene_manager;

			IRenderer() = delete;
			IRenderer(Engine* engine) : IModule(engine)
			{
			}
			virtual ~IRenderer() override = default;

			void Communicate(const ModuleMessage& data) override
			{
				switch (data.type)
				{
					case ModuleMessageType::RENDERER_TRANSFORMS:
					{
						const auto& processed_payload = std::get<RendererTransformUpdate>(data.payload);
						this->UpdateTransform(
							processed_payload.current,
							processed_payload.parent,
							processed_payload.screen
						);
						break;
					}
					case ModuleMessageType::RENDERER_SUPPLY:
					{
						const auto& processed_payload = std::get<RendererSupplyData>(data.payload);
						this->SupplyData(processed_payload);
						break;
					}
					default:
					{
						break;
					}
				}
			}

			virtual void UpdateMesh() = 0;

			virtual void Render(VkCommandBuffer commandBuffer, uint32_t currentFrame) = 0;

			[[nodiscard]] virtual bool GetIsUI() const = 0;
		};
	} // namespace Rendering
} // namespace Engine
