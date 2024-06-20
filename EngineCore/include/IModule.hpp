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

	enum class ModuleType
	{
		MIN_ENUM = 0x00,

		RENDERER = 5,

		MAX_ENUM = 0xFF
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
	};
} // namespace Engine
