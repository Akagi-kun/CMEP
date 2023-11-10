#pragma once

#include <functional>
#include <optional>

#include "glm/matrix.hpp"

#include "Logging/Logging.hpp"
#include "Rendering/IRenderer.hpp"
#include "PlatformSemantics.hpp"
#include "InternalEngineObject.hpp"

namespace Engine
{
	class CMEP_EXPORT Object : public InternalEngineObject
	{
	protected:
		/// <summary>
		/// Position of object in worldspace.
		/// </summary>
		glm::vec3 _pos = glm::vec3();

		/// <summary>
		/// Size of object.
		/// </summary>
		glm::vec3 _size = glm::vec3();

		/// <summary>
		/// Rotation of object.
		/// </summary>
		glm::vec3 _rotation = glm::vec3();

		/// <summary>
		/// Parent pos size and rot
		/// </summary>
		glm::vec3 _parent_pos = glm::vec3();
		glm::vec3 _parent_size = glm::vec3();
		glm::vec3 _parent_rotation = glm::vec3();

		Object* parent = nullptr;

		std::vector<Object*> children;

		unsigned int screenx = 0, screeny = 0;

		std::function<void(Object*)> _onClick = nullptr;

	public:
		Rendering::IRenderer* renderer = nullptr;
		
		Object() noexcept {}
		~Object() noexcept
		{
			this->logger->SimpleLog(Logging::LogLevel::Debug3, "Object deleted");
			delete this->renderer;
		}

		void ScreenSizeInform(unsigned int screenx, unsigned int screeny) noexcept
		{ 
			this->screenx = screenx;
			this->screeny = screeny;
			if (this->renderer != nullptr) { this->renderer->Update(this->_pos, this->_size, this->_rotation, this->screenx, this->screeny, this->_parent_pos, this->_parent_rotation, this->_parent_size); }
		}

		virtual void Translate(const glm::vec3 pos) noexcept
		{
			this->_pos = pos;
			if (this->renderer != nullptr) { this->renderer->Update(this->_pos, this->_size, this->_rotation, this->screenx, this->screeny, this->_parent_pos, this->_parent_rotation, this->_parent_size); }
			
			for(auto& child : this->children)
			{
				child->SetParentPositionRotationSize(this->_pos, this->_rotation, this->_size);
			 	child->UpdateRenderer();
			}
		}

		virtual void Scale(const glm::vec3 size) noexcept
		{ 
			this->_size = size; 
			if (this->renderer != nullptr) { this->renderer->Update(this->_pos, this->_size, this->_rotation, this->screenx, this->screeny, this->_parent_pos, this->_parent_rotation, this->_parent_size); }
		
			for(auto& child : this->children)
			{
				child->SetParentPositionRotationSize(this->_pos, this->_rotation, this->_size);
			 	child->UpdateRenderer();
			}
		}

		virtual void Rotate(const glm::vec3 rotation) noexcept 
		{ 
			this->_rotation = rotation;
			
			if (this->renderer != nullptr) { this->renderer->Update(this->_pos, this->_size, this->_rotation, this->screenx, this->screeny, this->_parent_pos, this->_parent_rotation, this->_parent_size); }
			
			for(auto& child : this->children)
			{
				child->SetParentPositionRotationSize(this->_pos, this->_rotation, this->_size);
			 	child->UpdateRenderer();
			}
		}

		virtual void UpdateRenderer() noexcept
		{
			if (this->renderer != nullptr) { this->renderer->Update(this->_pos, this->_size, this->_rotation, this->screenx, this->screeny, this->_parent_pos, this->_parent_rotation, this->_parent_size); }
		}

		virtual int Render(VkCommandBuffer commandBuffer, uint32_t currentFrame)
		{ 
			if (this->renderer != nullptr) { this->renderer->Render(commandBuffer, currentFrame); }
			return 0; 
		}

		glm::vec3 position() const noexcept { return this->_pos; }
		glm::vec3 size() const noexcept { return this->_size; }
		glm::vec3 rotation() const noexcept { return this->_rotation; }

		void SetParentPositionRotationSize(glm::vec3 position, glm::vec3 rotation, glm::vec3 size)
		{
			this->_parent_pos = position;
			this->_parent_rotation = rotation;
			this->_parent_size = size;
		}

		void AddChild(Object* object)
		{
			object->SetParent(this);
			object->SetParentPositionRotationSize(this->_pos, this->_rotation, this->_size);
			object->UpdateRenderer();
			this->children.push_back(object);
		}

		void RemoveChildren()
		{
			this->children.clear();
		}

		void SetParent(Object* object)
		{
			this->parent = object;
		}
	};
}