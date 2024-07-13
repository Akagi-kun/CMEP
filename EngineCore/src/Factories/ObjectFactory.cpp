#include "Factories/ObjectFactory.hpp"

#include "Rendering/IRenderer.hpp"
#include "Rendering/MeshRenderer.hpp"
#include "Rendering/SupplyData.hpp"

#include "Engine.hpp"

#include <cassert>
#include <memory>

namespace Engine::ObjectFactory
{
	Object* CreateGeneric3DObject(const std::shared_ptr<Scene>& scene, const std::shared_ptr<Rendering::Mesh>& mesh)
	{
		Engine* engine = scene->GetOwnerEngine();

		auto* object = new Object(engine);

		Rendering::IRenderer* with_renderer = new Rendering::MeshRenderer(engine);
		with_renderer->SupplyData({Rendering::RendererSupplyDataType::MESH, mesh});

		object->SetRenderer(with_renderer);

		return object;
	}
} // namespace Engine::ObjectFactory
