#pragma once

#include <stdint.h>
#include <string>
#include <map>
#include "PlatformSemantics.hpp"

namespace Engine
{
	class Engine;
}

namespace Engine::EventHandling
{
	enum class EventType
	{
		ON_INIT = 0x1,
		ON_UPDATE = 0x2,
		ON_KEYDOWN = 0x4,
		ON_KEYUP = 0x8,
		ON_MOUSEMOVED = 0x10,
		EVENT_UNDEFINED = 0xffff
	};

	class CMEP_EXPORT Event final
	{
	private:
	public:
		const EventType event_type;
		
		Engine* raisedFrom;

		double deltaTime = 0.0;
		union
		{
			uint16_t keycode = 0; // ON_KEYDOWN/ON_KEYUP events
			struct {
				double x;
				double y;
			} mouse; // ON_MOUSEMOVED event
		};

		Event(const EventType eventtype) : event_type(eventtype) {};
		~Event() {};
	};
}