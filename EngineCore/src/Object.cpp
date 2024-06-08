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
				this->_pos,
				this->_size,
				this->_rotation,
				this->screenx,
				this->screeny,
				this->_parent_pos,
				this->_parent_rotation,
				this->_parent_size
			);
		}
	}

	void Object::Translate(const glm::vec3 pos) noexcept
	{
		this->_pos = pos;
		if (this->renderer != nullptr)
		{
			this->renderer->Update(
				this->_pos,
				this->_size,
				this->_rotation,
				this->screenx,
				this->screeny,
				this->_parent_pos,
				this->_parent_rotation,
				this->_parent_size
			);
		}

		for (auto& child : this->children)
		{
			child->SetParentPositionRotationSize(this->_pos, this->_rotation, this->_size);
			child->UpdateRenderer();
		}
	}

	void Object::Scale(const glm::vec3 size) noexcept
	{
		this->_size = size;
		if (this->renderer != nullptr)
		{
			this->renderer->Update(
				this->_pos,
				this->_size,
				this->_rotation,
				this->screenx,
				this->screeny,
				this->_parent_pos,
				this->_parent_rotation,
				this->_parent_size
			);
		}

		for (auto& child : this->children)
		{
			child->SetParentPositionRotationSize(this->_pos, this->_rotation, this->_size);
			child->UpdateRenderer();
		}
	}

	void Object::Rotate(const glm::vec3 rotation) noexcept
	{
		this->_rotation = rotation;

		if (this->renderer != nullptr)
		{
			this->renderer->Update(
				this->_pos,
				this->_size,
				this->_rotation,
				this->screenx,
				this->screeny,
				this->_parent_pos,
				this->_parent_rotation,
				this->_parent_size
			);
		}

		for (auto& child : this->children)
		{
			child->SetParentPositionRotationSize(this->_pos, this->_rotation, this->_size);
			child->UpdateRenderer();
		}
	}

	void Object::UpdateRenderer() noexcept
	{
		if (this->renderer != nullptr)
		{
			this->renderer->Update(
				this->_pos,
				this->_size,
				this->_rotation,
				this->screenx,
				this->screeny,
				this->_parent_pos,
				this->_parent_rotation,
				this->_parent_size
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

	glm::vec3 Object::position() const noexcept
	{
		return this->_pos;
	}
	glm::vec3 Object::size() const noexcept
	{
		return this->_size;
	}
	glm::vec3 Object::rotation() const noexcept
	{
		return this->_rotation;
	}

	void Object::SetParentPositionRotationSize(glm::vec3 position, glm::vec3 rotation, glm::vec3 size)
	{
		this->_parent_pos = position;
		this->_parent_rotation = rotation;
		this->_parent_size = size;
	}

	void Object::AddChild(Object* object)
	{
		object->SetParent(this);
		object->SetParentPositionRotationSize(this->_pos, this->_rotation, this->_size);
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