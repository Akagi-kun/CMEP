#pragma once

#include "Rendering/SupplyData.hpp"

#include "InternalEngineObject.hpp"

#include <variant>

namespace Engine
{
	enum class ModuleMessageType
	{
		MIN_ENUM = 0x00,

		RENDERER_SUPPLY		= 1,
		RENDERER_TRANSFORMS = 2,

		MAX_ENUM = 0xFF
	};

	enum class ModuleMessageTarget
	{
		MIN_ENUM = 0x00,

		RENDERER = 5,

		MAX_ENUM = 0xFF
	};

	struct ModuleMessage
	{
		ModuleMessageTarget target;
		ModuleMessageType type;
		std::variant<Rendering::RendererSupplyData, Rendering::RendererTransformUpdate> payload;
	};

	class IModule : public InternalEngineObject
	{
	private:
	public:
		IModule() = delete;
		IModule(Engine* engine) : InternalEngineObject(engine)
		{
		}
		virtual ~IModule() = default;

		virtual void Communicate(const ModuleMessage& data) = 0;
	};
} // namespace Engine
