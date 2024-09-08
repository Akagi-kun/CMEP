#pragma once

#include "Logging/Logging.hpp"

#include <memory>

namespace Engine
{
	class Engine;

	class InternalEngineObject : public Logging::SupportsLogging
	{
	public:
		InternalEngineObject() = delete;
		InternalEngineObject(Engine* with_engine);

		Engine* getOwnerEngine()
		{
			return this->owner_engine;
		}

		std::weak_ptr<Logging::Logger> getLogger()
		{
			return this->logger;
		}

	protected:
		Engine* owner_engine = nullptr;
	};
} // namespace Engine
