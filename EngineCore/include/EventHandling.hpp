#pragma once

#include "EnumStringConvertor.hpp"

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

		ON_INIT		  = 0x1,
		ON_UPDATE	  = 0x2,
		ON_KEYDOWN	  = 0x4,
		ON_KEYUP	  = 0x8,
		ON_MOUSEMOVED = 0x10,

		MAX_ENUM = 0xFF,
	};

	// TODO: Make this a struct
	class Event final
	{
	private:
	public:
		const EventType event_type;
		Engine* const raised_from = nullptr;

		double delta_time = 0.0;
		union {
			uint16_t keycode = 0; // ON_KEYDOWN/ON_KEYUP events
			struct
			{
				double x;
				double y;
			} mouse; // ON_MOUSEMOVED event
		};

		Event(Engine* const with_engine, const EnumStringConvertor<EventType> eventtype)
			: event_type(eventtype), raised_from(with_engine)
		{
		}
		~Event() = default;
	};
} // namespace Engine::EventHandling
