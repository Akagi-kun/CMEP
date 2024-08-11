#pragma once

#include "Logging/Logging.hpp"

#include <memory>
#include <utility>

namespace Engine
{
	class Engine;

	class SupportsLogging
	{
	public:
		using logger_t = std::shared_ptr<Logging::Logger>;

		SupportsLogging() = delete;
		SupportsLogging(logger_t with_logger) : logger(std::move(with_logger))
		{
		}

	protected:
		logger_t logger;
	};

	class InternalEngineObject : public SupportsLogging
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
