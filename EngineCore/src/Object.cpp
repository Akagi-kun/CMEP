#include "Object.hpp"

namespace Engine
{
	Object::Object() noexcept
	{
	}
	Object::~Object() noexcept
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug3, "Object deleted");
		delete this->renderer;
	}

	void Object::ScreenSizeInform(unsigned int screenx, unsigned int screeny) noexcept
	{
		this->screenx = screenx;
		this->screeny = screeny;
		if (this->renderer != nullptr)
		{
			this->renderer->Update(
				this->pos,
				this->size,
				this->rotation,
				this->screenx,
				this->screeny,
				this->parent_pos,
				this->parent_rotation,
				this->parent_size
			);
		}
	}

	void Object::Translate(const glm::vec3 pos) noexcept
	{
		this->pos = pos;
		if (this->renderer != nullptr)
		{
			this->renderer->Update(
				this->pos,
				this->size,
				this->rotation,
				this->screenx,
				this->screeny,
				this->parent_pos,
				this->parent_rotation,
				this->parent_size
			);
		}

		for (auto& child : this->children)
		{
			child->SetParentPositionRotationSize(this->pos, this->rotation, this->size);
			child->UpdateRenderer();
		}
	}

	void Object::Scale(const glm::vec3 size) noexcept
	{
		this->size = size;
		if (this->renderer != nullptr)
		{
			this->renderer->Update(
				this->pos,
				this->size,
				this->rotation,
				this->screenx,
				this->screeny,
				this->parent_pos,
				this->parent_rotation,
				this->parent_size
			);
		}

		for (auto& child : this->children)
		{
			child->SetParentPositionRotationSize(this->pos, this->rotation, this->size);
			child->UpdateRenderer();
		}
	}

	void Object::Rotate(const glm::vec3 rotation) noexcept
	{
		this->rotation = rotation;

		if (this->renderer != nullptr)
		{
			this->renderer->Update(
				this->pos,
				this->size,
				this->rotation,
				this->screenx,
				this->screeny,
				this->parent_pos,
				this->parent_rotation,
				this->parent_size
			);
		}

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
			this->renderer->Update(
				this->pos,
				this->size,
				this->rotation,
				this->screenx,
				this->screeny,
				this->parent_pos,
				this->parent_rotation,
				this->parent_size
			);
		}
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

	void Object::SetParentPositionRotationSize(glm::vec3 position, glm::vec3 rotation, glm::vec3 size)
	{
		this->parent_pos = position;
		this->parent_rotation = rotation;
		this->parent_size = size;
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