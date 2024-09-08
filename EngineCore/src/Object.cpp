#include "Object.hpp"

#include "Rendering/MeshBuilders/IMeshBuilder.hpp"
#include "Rendering/Renderers/Renderer.hpp"
#include "Rendering/Transform.hpp"

#include "Logging/Logging.hpp"

#include "InternalEngineObject.hpp"

namespace Engine
{
	using Rendering::IMeshBuilder;
	using Rendering::IRenderer;

	Object::Object(Engine* with_engine, IRenderer* with_renderer, IMeshBuilder* with_mesh_builder)
		: InternalEngineObject(with_engine), renderer(with_renderer),
		  mesh_builder(with_mesh_builder)
	{
	}

	Object::~Object() noexcept
	{
		this->logger->simpleLog<decltype(this)>(
			Logging::LogLevel::VerboseDebug,
			"Destructor called"
		);

		delete renderer;
	}

	void Object::updateRenderer()
	{
		if (renderer != nullptr)
		{
			renderer->updateTransform(transform, parent_transform, screen);
		}
	}

	void Object::screenSizeInform(Rendering::ScreenSize with_screen_size)
	{
		screen = with_screen_size;

		updateRenderer();
	}

	void Object::setPosition(const glm::vec3 with_pos)
	{
		transform.pos = with_pos;
		updateRenderer();

		for (auto& child : children)
		{
			child->setParentTransform(transform);
			child->updateRenderer();
		}
	}

	void Object::setSize(const glm::vec3 with_size)
	{
		transform.size = with_size;
		updateRenderer();

		for (auto& child : children)
		{
			child->setParentTransform(transform);
			child->updateRenderer();
		}
	}

	void Object::setRotation(const glm::vec3 with_rotation)
	{
		transform.rotation = with_rotation;

		updateRenderer();

		for (auto& child : children)
		{
			child->setParentTransform(transform);
			child->updateRenderer();
		}
	}

	glm::vec3 Object::getPosition() const noexcept
	{
		return transform.pos;
	}
	glm::vec3 Object::getSize() const noexcept
	{
		return transform.size;
	}
	glm::vec3 Object::getRotation() const noexcept
	{
		return transform.rotation;
	}

	void Object::setParentTransform(Rendering::Transform with_parent_transform)
	{
		parent_transform = with_parent_transform;
	}

	void Object::addChild(Object* with_child)
	{
		with_child->setParent(this);
		with_child->setParentTransform(transform);
		with_child->updateRenderer();
		children.push_back(with_child);
	}

	void Object::removeChildren()
	{
		children.clear();
	}

	void Object::setParent(Object* with_parent)
	{
		parent = with_parent;
	}
} // namespace Engine
