#include "Object.hpp"

#include "Rendering/Renderers/Renderer.hpp"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_OBJECT
#include "Logging/LoggingPrefix.hpp" // IWYU pragma: keep

namespace Engine
{
	Object::~Object() noexcept
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Destructor called");

		delete this->renderer;
	}

	void Object::SetRenderer(Rendering::IRenderer* with_renderer)
	{
		assert(this->owner_engine != nullptr);

		this->renderer = with_renderer;
	}

	Rendering::IRenderer* Object::GetRenderer()
	{
		return this->renderer;
	}

	void Object::UpdateRenderer() noexcept
	{
		if (this->renderer != nullptr)
		{
			this->renderer->UpdateTransform(this->transform, this->parent_transform, this->screen);
		}
	}

	void Object::ScreenSizeInform(unsigned int with_screenx, unsigned int with_screeny) noexcept
	{
		this->screen.x = with_screenx;
		this->screen.y = with_screeny;

		this->UpdateRenderer();
	}

	void Object::SetPosition(const glm::vec3 with_pos) noexcept
	{
		this->transform.pos = with_pos;
		this->UpdateRenderer();

		for (auto& child : this->children)
		{
			child->SetParentTransform(this->transform);
			child->UpdateRenderer();
		}
	}

	void Object::SetSize(const glm::vec3 with_size) noexcept
	{
		this->transform.size = with_size;
		this->UpdateRenderer();

		for (auto& child : this->children)
		{
			child->SetParentTransform(this->transform);
			child->UpdateRenderer();
		}
	}

	void Object::SetRotation(const glm::vec3 with_rotation) noexcept
	{
		this->transform.rotation = with_rotation;

		this->UpdateRenderer();

		for (auto& child : this->children)
		{
			child->SetParentTransform(this->transform);
			child->UpdateRenderer();
		}
	}

	glm::vec3 Object::GetPosition() const noexcept
	{
		return this->transform.pos;
	}
	glm::vec3 Object::GetSize() const noexcept
	{
		return this->transform.size;
	}
	glm::vec3 Object::GetRotation() const noexcept
	{
		return this->transform.rotation;
	}

	void Object::SetParentTransform(Rendering::Transform with_parent_transform)
	{
		this->parent_transform = with_parent_transform;
	}

	void Object::AddChild(Object* with_child)
	{
		with_child->SetParent(this);
		with_child->SetParentTransform(this->transform);
		with_child->UpdateRenderer();
		this->children.push_back(with_child);
	}

	void Object::RemoveChildren()
	{
		this->children.clear();
	}

	void Object::SetParent(Object* with_parent)
	{
		this->parent = with_parent;
	}
} // namespace Engine
