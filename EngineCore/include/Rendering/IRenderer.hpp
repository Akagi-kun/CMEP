#pragma once

#include "glm/vec3.hpp"

#include "InternalEngineObject.hpp"
#include "PlatformSemantics.hpp"
#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"

#include "SupplyData.hpp"

namespace Engine
{
	class Object;
	class Engine;
	class SceneManager;

	namespace Rendering
	{
		class Shader;

		/**
		 * @brief Interface for Renderers
		 */
		class IRenderer : public InternalEngineObject
		{
		protected:
			glm::vec3 pos = glm::vec3();
			glm::vec3 size = glm::vec3();
			glm::vec3 rotation = glm::vec3();

			glm::vec3 parent_pos = glm::vec3();
			glm::vec3 parent_size = glm::vec3();
			glm::vec3 parent_rotation = glm::vec3();

			uint_fast16_t screenx = 0, screeny = 0;

			bool has_updated_mesh = false;

		public:
			std::weak_ptr<::Engine::SceneManager> scene_manager{};

			IRenderer() = delete;
			IRenderer(Engine* engine) : InternalEngineObject(engine) {};
			virtual ~IRenderer() {};

			virtual void Update(
				glm::vec3 pos,
				glm::vec3 size,
				glm::vec3 rotation,
				uint_fast16_t screenx,
				uint_fast16_t screeny,
				glm::vec3 parent_position,
				glm::vec3 parent_rotation,
				glm::vec3 parent_size
			) = 0;

			virtual void SupplyData(RendererSupplyData data) = 0;

			virtual void UpdateMesh() = 0;

			virtual void Render(VkCommandBuffer commandBuffer, uint32_t currentFrame) = 0;

			virtual bool GetIsUI() const = 0;
		};
	} // namespace Rendering
} // namespace Engine