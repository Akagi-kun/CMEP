#pragma once

#include "Rendering/SupplyData.hpp"

#include "InternalEngineObject.hpp"

#include <variant>

namespace Engine
{
	enum class ModuleMessageType
	{
		MIN_ENUM = 0x00,

		RENDERER_DATA = 1,
		RENDERER_UPDATE = 2,

		MAX_ENUM = 0xFF
	};

	struct ModuleMessage
	{
		ModuleMessageType type;
		std::variant<Rendering::RendererSupplyData> payload;
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
