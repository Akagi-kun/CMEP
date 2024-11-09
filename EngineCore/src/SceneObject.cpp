#include "SceneObject.hpp"

#include "Rendering/MeshBuilders/IMeshBuilder.hpp"
#include "Rendering/Renderers/Renderer.hpp"
#include "Rendering/Transform.hpp"

#include "Logging/Logging.hpp"

#include "InternalEngineObject.hpp"

#include <cassert>

namespace Engine
{
	using Rendering::IMeshBuilder;
	using Rendering::IRenderer;

	SceneObject::SceneObject(
		Engine*		  with_engine,
		IRenderer*	  with_renderer,
		IMeshBuilder* with_mesh_builder
	)
		: InternalEngineObject(with_engine), renderer(with_renderer),
		  mesh_builder(with_mesh_builder)
	{}

	SceneObject::~SceneObject() noexcept
	{
		this->logger->logSingle<decltype(this)>(Logging::LogLevel::VerboseDebug, "Destructor called");

		delete renderer;
	}

	void SceneObject::updateRenderer()
	{
		assert(renderer);

		renderer->updateTransform(transform, parent_transform);
	}

	void SceneObject::setPosition(const glm::vec3& with_pos)
	{
		transform.pos = with_pos;
		updateRenderer();

		for (auto& child : children)
		{
			child->setParentTransform(transform);
			child->updateRenderer();
		}
	}

	void SceneObject::setSize(const glm::vec3& with_size)
	{
		transform.size = with_size;
		updateRenderer();

		for (auto& child : children)
		{
			child->setParentTransform(transform);
			child->updateRenderer();
		}
	}

	void SceneObject::setRotation(const glm::vec3& with_rotation)
	{
		transform.rotation = with_rotation;

		updateRenderer();

		for (auto& child : children)
		{
			child->setParentTransform(transform);
			child->updateRenderer();
		}
	}

	glm::vec3 SceneObject::getPosition() const noexcept
	{
		return transform.pos;
	}
	glm::vec3 SceneObject::getSize() const noexcept
	{
		return transform.size;
	}
	glm::vec3 SceneObject::getRotation() const noexcept
	{
		return transform.rotation;
	}

	void SceneObject::setParentTransform(const Rendering::Transform& with_parent_transform)
	{
		parent_transform = with_parent_transform;
	}

	void SceneObject::addChild(SceneObject* with_child)
	{
		with_child->setParent(this);
		with_child->setParentTransform(transform);
		with_child->updateRenderer();

		children.push_back(with_child);
	}

	void SceneObject::removeChildren()
	{
		children.clear();
	}

	void SceneObject::setParent(SceneObject* with_parent)
	{
		parent = with_parent;
	}
} // namespace Engine
