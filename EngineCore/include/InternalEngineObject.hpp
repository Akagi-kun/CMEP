#pragma once

#include "Logging/Logging.hpp"

#include <memory>

namespace Engine
{
	class Engine;

	class InternalEngineObject
	{
	protected:
		std::shared_ptr<Logging::Logger> logger;
		Engine* owner_engine = nullptr;

	public:
		InternalEngineObject() = delete;
		InternalEngineObject(Engine* with_engine);

		Engine* GetOwnerEngine()
		{
			return this->owner_engine;
		}

		std::weak_ptr<Logging::Logger> GetLogger()
		{
			return this->logger;
		}
	};
} // namespace Engine
