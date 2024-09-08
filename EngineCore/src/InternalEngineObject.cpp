#include "InternalEngineObject.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"

namespace Engine
{
	InternalEngineObject::InternalEngineObject(Engine* with_engine)
		: SupportsLogging(with_engine->getLogger()), owner_engine(with_engine)
	{
	}
} // namespace Engine
