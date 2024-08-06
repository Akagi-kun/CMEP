#include "Rendering/Renderer2D.hpp"

#include "Assets/Font.hpp"
#include "Assets/Texture.hpp"
#include "Rendering/IRenderer.hpp"
#include "Rendering/framework.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"

#include <cassert>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Engine::Rendering
{
	/* Renderer2D::Renderer2D(Engine* engine, IMeshBuilder* with_builder, std::string_view with_pipeline_program)
		: IRenderer(engine, with_builder, with_pipeline_program)
	{
	} */

	void Renderer2D::SupplyData(const RendererSupplyData& data)
	{
		switch (data.type)
		{
			case RendererSupplyDataType::FONT:
			{
				const auto& payload_ref = std::get<std::weak_ptr<void>>(data.payload);
				assert(!payload_ref.expired() && "Cannot lock expired payload!");

				auto font_cast				  = std::static_pointer_cast<Font>(payload_ref.lock());
				this->texture				  = font_cast->GetPageTexture(0);
				this->has_updated_descriptors = false;
				break;
			}
			case RendererSupplyDataType::TEXTURE:
			{
				const auto& payload_ref = std::get<std::weak_ptr<void>>(data.payload);
				assert(!payload_ref.expired() && "Cannot lock expired payload!");

				this->texture				  = std::static_pointer_cast<Texture>(payload_ref.lock());
				this->has_updated_descriptors = false;
				break;
			}
			default:
			{
				break;
			}
		}

		assert(this->mesh_builder != nullptr && "This renderer has not been assigned a mesh builder!");
		this->mesh_builder->SupplyData(data);
	}

	void Renderer2D::UpdateMatrices()
	{
		glm::mat4 projection{};
		if (auto locked_scene_manager = this->owner_engine->GetSceneManager().lock())
		{
			projection = locked_scene_manager->GetProjectionMatrixOrtho();
		}

		this->matrix_data.mat_model = CalculateModelMatrix(this->transform, this->parent_transform);
		this->matrix_data.mat_vp	= projection; // * view;

		this->has_updated_matrices = true;
	}
} // namespace Engine::Rendering
