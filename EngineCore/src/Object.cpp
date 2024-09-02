#include "Object.hpp"

#include "Rendering/Renderers/Renderer.hpp"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_OBJECT
#include "Logging/LoggingPrefix.hpp"

namespace Engine
{
	Object::Object(
		Engine* with_engine,
		Rendering::IRenderer* with_renderer,
		Rendering::IMeshBuilder* with_mesh_builder
	)
		: InternalEngineObject(with_engine), renderer(with_renderer),
		  mesh_builder(with_mesh_builder)
	{
	}

	Object::~Object() noexcept
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Destructor called");

		delete renderer;
	}

	void Object::UpdateRenderer()
	{
		if (renderer != nullptr)
		{
			renderer->UpdateTransform(transform, parent_transform, screen);
		}
	}

	void Object::ScreenSizeInform(Rendering::ScreenSize with_screen_size)
	{
		screen = with_screen_size;

		UpdateRenderer();
	}

	void Object::SetPosition(const glm::vec3 with_pos)
	{
		transform.pos = with_pos;
		UpdateRenderer();

		for (auto& child : children)
		{
			child->SetParentTransform(transform);
			child->UpdateRenderer();
		}
	}

	void Object::SetSize(const glm::vec3 with_size)
	{
		transform.size = with_size;
		UpdateRenderer();

		for (auto& child : children)
		{
			child->SetParentTransform(transform);
			child->UpdateRenderer();
		}
	}

	void Object::SetRotation(const glm::vec3 with_rotation)
	{
		transform.rotation = with_rotation;

		UpdateRenderer();

		for (auto& child : children)
		{
			child->SetParentTransform(transform);
			child->UpdateRenderer();
		}
	}

	glm::vec3 Object::GetPosition() const noexcept
	{
		return transform.pos;
	}
	glm::vec3 Object::GetSize() const noexcept
	{
		return transform.size;
	}
	glm::vec3 Object::GetRotation() const noexcept
	{
		return transform.rotation;
	}

	void Object::SetParentTransform(Rendering::Transform with_parent_transform)
	{
		parent_transform = with_parent_transform;
	}

	void Object::AddChild(Object* with_child)
	{
		with_child->SetParent(this);
		with_child->SetParentTransform(transform);
		with_child->UpdateRenderer();
		children.push_back(with_child);
	}

	void Object::RemoveChildren()
	{
		children.clear();
	}

	void Object::SetParent(Object* with_parent)
	{
		parent = with_parent;
	}
} // namespace Engine
