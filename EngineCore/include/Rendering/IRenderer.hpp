#pragma once

#include "Rendering/Transform.hpp"
#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"

#include "IModule.hpp"
#include "InternalEngineObject.hpp"
#include "SupplyData.hpp"
#include "Transform.hpp"
#include "glm/glm.hpp"

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

		public:
			std::weak_ptr<::Engine::SceneManager> scene_manager{};

			IRenderer() = delete;
			IRenderer(Engine* engine) : IModule(engine)
			{
			}
			virtual ~IRenderer() override = default;

			virtual void UpdateTransform(
				Transform with_transform,
				Transform with_parent_transform,
				ScreenSize with_screen
			)
			{
				this->transform = with_transform;
				this->parent_transform = with_parent_transform;
				this->screen = with_screen;

				this->has_updated_mesh = false;
			}

			void Communicate(const ModuleMessage& data) override
			{
			}

			virtual void SupplyData(RendererSupplyData data) = 0;

			virtual void UpdateMesh() = 0;

			virtual void Render(VkCommandBuffer commandBuffer, uint32_t currentFrame) = 0;

			virtual bool GetIsUI() const = 0;
		};
	} // namespace Rendering
} // namespace Engine
