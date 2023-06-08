#pragma once

#include <stdint.h>

namespace Engine::EventHandling
{
	enum class EventType
	{
		ON_INIT,
		ON_UPDATE,
		ON_KEYDOWN,
		ON_MOUSEMOVED
	};

	class __declspec(dllexport) Event final
	{
	private:
	public:
		const EventType event_type;

		double deltaTime = 0.0;
		union
		{
			uint16_t keycode = 0; // ON_KEYDOWN event
			struct {
				double x;
				double y;
			} mouse; // ON_MOUSEMOVED event
		};

		Event(const EventType eventtype) : event_type(eventtype) {};
		~Event() {};
	};
}