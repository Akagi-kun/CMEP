#include "Rendering/MeshBuilders/IMeshBuilder.hpp"

#include "Engine.hpp"

namespace Engine::Rendering
{
	IMeshBuilder::IMeshBuilder(Engine* engine) : InternalEngineObject(engine), renderer(engine->GetRenderingEngine())
	{
	}
} // namespace Engine::Rendering
