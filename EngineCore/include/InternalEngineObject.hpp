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
		InternalEngineObject() = default;
		InternalEngineObject(Engine* engine) : owner_engine(engine)
		{
		}

		void UpdateOwnerEngine(Engine* new_owner_engine)
		{
			this->owner_engine = new_owner_engine;
		}

		void UpdateHeldLogger(std::shared_ptr<Logging::Logger>& new_logger)
		{
			this->logger = new_logger;
		}

		// Const qualify this?
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
