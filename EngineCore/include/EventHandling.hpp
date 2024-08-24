#pragma once

// #include "DataTypes.hpp"
#include "EnumStringConvertor.hpp"
#include "glm/glm.hpp"

#include <cstdint>

namespace Engine
{
	class Engine;
}

namespace Engine::EventHandling
{
	enum class EventType : uint8_t
	{
		MIN_ENUM = 0x00,

		ON_INIT	  = 1,
		ON_UNLOAD = 2,

		ON_UPDATE = 8,

		ON_KEYDOWN = 16,
		ON_KEYUP   = 18,

		ON_MOUSEMOVED = 24,

		MAX_ENUM = 0xFF,
	};

	struct Event final
	{
		const EventType event_type;
		Engine* const raised_from = nullptr;

		// delta time shall be specified for every event 
		double delta_time = 0.0;
		union { // event specific data
			uint16_t keycode = 0;	   // ON_KEYDOWN/ON_KEYUP events
			glm::vec<2, double> mouse; // ON_MOUSEMOVED event
		};

		Event(Engine* const with_engine, const EnumStringConvertor<EventType> eventtype)
			: event_type(eventtype), raised_from(with_engine)
		{
		}
		~Event() = default;
	};
} // namespace Engine::EventHandling
