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

		for (auto& module : this->modules)
		{
			delete module.second;
		}

		this->modules.clear();
		// delete this->renderer;
	}

	void Object::ModuleBroadcast(ModuleType for_type, const ModuleMessage& data)
	{
		auto module_range = this->modules.equal_range(for_type);
		for (auto module = module_range.first; module != module_range.second; ++module)
		{
			module->second->Communicate(data);
		}
	}

	void Object::AddModule(ModuleType with_type, IModule* with_module)
	{
		with_module->SetMediator(this);

		this->modules.emplace(with_type, with_module);
	}

	void Object::UpdateRenderer() noexcept
	{
		ModuleMessage message = {
			ModuleMessageType::RENDERER_TRANSFORMS,
			Rendering::RendererTransformUpdate{this->transform, this->parent_transform, this->screen}
		};
		this->ModuleBroadcast(ModuleType::RENDERER, message);
	}

	void Object::UpdateHeldLogger(std::shared_ptr<Logging::Logger>& new_logger)
	{
		InternalEngineObject::UpdateHeldLogger(new_logger);

		for (auto& module : this->modules)
		{
			module.second->UpdateHeldLogger(new_logger);
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
			child->SetParentPositionRotationSize(this->transform);
			child->UpdateRenderer();
		}
	}

	void Object::SetSize(const glm::vec3 with_size) noexcept
	{
		this->transform.size = with_size;
		this->UpdateRenderer();

		for (auto& child : this->children)
		{
			child->SetParentPositionRotationSize(this->transform);
			child->UpdateRenderer();
		}
	}

	void Object::SetRotation(const glm::vec3 with_rotation) noexcept
	{
		this->transform.rotation = with_rotation;

		this->UpdateRenderer();

		for (auto& child : this->children)
		{
			child->SetParentPositionRotationSize(this->transform);
			child->UpdateRenderer();
		}
	}

	IModule* Object::GetFirstModule(ModuleType with_type)
	{
		auto module_range = this->modules.equal_range(with_type);

		// Returns first module of this type
		return module_range.first->second;
	}

	/*
	   int Object::Render(VkCommandBuffer commandBuffer, uint32_t currentFrame)
	   {
		   if (this->renderer != nullptr)
		   {
			   static_cast<Rendering::IRenderer*>(this->renderer)->Render(commandBuffer, currentFrame);
		   }
		   return 0;
	   }
	*/
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

	void Object::SetParent(Object* with_parent)
	{
		this->parent = with_parent;
	}
} // namespace Engine
