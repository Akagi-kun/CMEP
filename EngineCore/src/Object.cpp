#include "Object.hpp"

#include "Rendering/IRenderer.hpp"

#include "InternalEngineObject.hpp"

namespace Engine
{
	Object::Object() noexcept = default;
	Object::~Object() noexcept
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug3, "Object deleted");
		delete this->renderer;
	}

	void Object::UpdateHeldLogger(std::shared_ptr<Logging::Logger> new_logger)
	{
		InternalEngineObject::UpdateHeldLogger(new_logger);

		if (this->renderer != nullptr)
		{
			this->renderer->UpdateHeldLogger(new_logger);
		}
	}

	void Object::ScreenSizeInform(unsigned int new_screenx, unsigned int newscreeny) noexcept
	{
		this->screenx = new_screenx;
		this->screeny = newscreeny;

		this->UpdateRenderer();
	}

	void Object::Translate(const glm::vec3 new_pos) noexcept
	{
		this->pos = new_pos;

		this->UpdateRenderer();

		for (auto& child : this->children)
		{
			child->SetParentPositionRotationSize(this->pos, this->rotation, this->size);
			child->UpdateRenderer();
		}
	}

	void Object::Scale(const glm::vec3 new_size) noexcept
	{
		this->size = new_size;

		this->UpdateRenderer();

		for (auto& child : this->children)
		{
			child->SetParentPositionRotationSize(this->pos, this->rotation, this->size);
			child->UpdateRenderer();
		}
	}

	void Object::Rotate(const glm::vec3 new_rotation) noexcept
	{
		this->rotation = new_rotation;

		this->UpdateRenderer();

		for (auto& child : this->children)
		{
			child->SetParentPositionRotationSize(this->pos, this->rotation, this->size);
			child->UpdateRenderer();
		}
	}

	void Object::UpdateRenderer() noexcept
	{
		if (this->renderer != nullptr)
		{
			this->renderer->UpdateTransform(
				{this->pos, this->size, this->rotation},
				{this->parent_pos, this->parent_size, this->parent_rotation},
				{this->screenx, this->screeny}
			);
		}
	}

	Rendering::IRenderer* Object::AssignRenderer(Rendering::IRenderer* with_renderer)
	{
		Rendering::IRenderer* old_renderer = this->renderer;

		this->renderer = with_renderer;

		return old_renderer;
	}

	Rendering::IRenderer* Object::GetRenderer() noexcept
	{
		return this->renderer;
	}

	int Object::Render(VkCommandBuffer commandBuffer, uint32_t currentFrame)
	{
		if (this->renderer != nullptr)
		{
			this->renderer->Render(commandBuffer, currentFrame);
		}
		return 0;
	}

	glm::vec3 Object::Position() const noexcept
	{
		return this->pos;
	}
	glm::vec3 Object::Size() const noexcept
	{
		return this->size;
	}
	glm::vec3 Object::Rotation() const noexcept
	{
		return this->rotation;
	}

	void Object::SetParentPositionRotationSize(glm::vec3 new_position, glm::vec3 new_rotation, glm::vec3 new_size)
	{
		this->parent_pos = new_position;
		this->parent_rotation = new_rotation;
		this->parent_size = new_size;
	}

	void Object::AddChild(Object* object)
	{
		object->SetParent(this);
		object->SetParentPositionRotationSize(this->pos, this->rotation, this->size);
		object->UpdateRenderer();
		this->children.push_back(object);
	}

	void Object::RemoveChildren()
	{
		this->children.clear();
	}

	void Object::SetParent(Object* object)
	{
		this->parent = object;
	}
} // namespace Engine
