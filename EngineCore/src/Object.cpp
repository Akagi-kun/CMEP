#include "Object.hpp"

#include "Rendering/IRenderer.hpp"

#include "IModule.hpp"
#include "InternalEngineObject.hpp"

// Prefixes for logging messages
#define LOGPFX_CURRENT LOGPFX_CLASS_OBJECT
#include "Logging/LoggingPrefix.hpp"

namespace Engine
{
	Object::Object() noexcept = default;
	Object::~Object() noexcept
	{
		this->logger->SimpleLog(Logging::LogLevel::Debug3, LOGPFX_CURRENT "Destructor called");
		delete this->renderer;
	}

	void Object::UpdateRenderer() noexcept
	{
		if (this->renderer != nullptr)
		{
			ModuleMessage message = {
				ModuleMessageType::RENDERER_TRANSFORMS,
				Rendering::RendererTransformUpdate{this->transform, this->parent_transform, this->screen}
			};
			this->renderer->Communicate(message);
		}
	}

	void Object::UpdateHeldLogger(std::shared_ptr<Logging::Logger> new_logger)
	{
		InternalEngineObject::UpdateHeldLogger(new_logger);

		if (this->renderer != nullptr)
		{
			this->renderer->UpdateHeldLogger(new_logger);
		}
	}

	void Object::ScreenSizeInform(unsigned int with_screenx, unsigned int with_screeny) noexcept
	{
		this->screen.x = with_screenx;
		this->screen.y = with_screeny;

		this->UpdateRenderer();
	}

	void Object::Translate(const glm::vec3 new_pos) noexcept
	{
		this->transform.pos = new_pos;
		this->UpdateRenderer();

		for (auto& child : this->children)
		{
			child->SetParentPositionRotationSize(this->transform);
			child->UpdateRenderer();
		}
	}

	void Object::Scale(const glm::vec3 new_size) noexcept
	{
		this->transform.size = new_size;
		this->UpdateRenderer();

		for (auto& child : this->children)
		{
			child->SetParentPositionRotationSize(this->transform);
			child->UpdateRenderer();
		}
	}

	void Object::Rotate(const glm::vec3 new_rotation) noexcept
	{
		this->transform.rotation = new_rotation;

		this->UpdateRenderer();

		for (auto& child : this->children)
		{
			child->SetParentPositionRotationSize(this->transform);
			child->UpdateRenderer();
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
		return this->transform.pos;
	}
	glm::vec3 Object::Size() const noexcept
	{
		return this->transform.size;
	}
	glm::vec3 Object::Rotation() const noexcept
	{
		return this->transform.rotation;
	}

	void Object::SetParentPositionRotationSize(Rendering::Transform with_parent_transform)
	{
		this->parent_transform = with_parent_transform;
	}

	void Object::AddChild(Object* with_child)
	{
		with_child->SetParent(this);
		with_child->SetParentPositionRotationSize(this->transform);
		with_child->UpdateRenderer();
		this->children.push_back(with_child);
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
