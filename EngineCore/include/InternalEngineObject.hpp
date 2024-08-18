#pragma once

#include "Logging/Logging.hpp"

#include <memory>
#include <utility>

namespace Engine
{
	class Engine;

	class InternalEngineObject : public Logging::SupportsLogging
	{
	protected:
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
