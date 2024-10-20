#pragma once

#include "glm/vec2.hpp"

#include <cstdint>

namespace Engine
{
	class Engine;
}

namespace Engine::EventHandling
{
	enum class EventType : uint8_t
	{
		onInit	 = 1,
		onUnload = 2, // currently unused

		onUpdate = 8,

		onKeyDown = 16,
		onKeyUp	  = 18,

		onMouseMoved = 24,

		minEnum = 0x00,
		maxEnum = 0xFF,
	};

	struct Event final
	{
		const EventType event_type;
		Engine* const	raised_from = nullptr;

		// delta time shall be specified for every event
		double delta_time = 0.0;
		union {						// event specific data
			uint16_t   keycode = 0; // onKeyDown/onKeyUp events
			glm::dvec2 mouse;		// onMouseMoved event
		};

		Event(Engine* const with_engine, EventType eventtype)
			: event_type(eventtype), raised_from(with_engine)
		{}
	};
} // namespace Engine::EventHandling
