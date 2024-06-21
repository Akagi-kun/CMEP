#pragma once

#include "Rendering/SupplyData.hpp"

#include "InternalEngineObject.hpp"

#include <variant>

namespace Engine
{
	enum class ModuleMessageType : uint8_t
	{
		MIN_ENUM = 0x00,

		RENDERER_SUPPLY		= 1,
		RENDERER_TRANSFORMS = 2,
		RENDERER_REQ_RENDER = 5,

		MAX_ENUM = 0xFF
	};

	enum class ModuleType : uint8_t
	{
		MIN_ENUM = 0x00,

		RENDERER	 = 5,
		MESH_BUILDER = 20,

		MAX_ENUM = 0xFF
	};

	struct ModuleMessage
	{
		ModuleMessageType type;
		std::
			variant<Rendering::RendererSupplyData, Rendering::RendererTransformUpdate, Rendering::RendererRenderRequest>
				payload;
	};

	class IModule : public InternalEngineObject
	{
	protected:
		ModuleType type;

	public:
		IModule() = delete;
		IModule(Engine* engine, ModuleType as_type) : InternalEngineObject(engine), type(as_type)
		{
		}
		virtual ~IModule() = default;

		virtual void Communicate(const ModuleMessage& data) = 0;

		[[nodiscard]] ModuleType GetType() const
		{
			return this->type;
		}
	};
} // namespace Engine
