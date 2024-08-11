#include "InternalEngineObject.hpp"

#include "Engine.hpp"

namespace Engine
{
	InternalEngineObject::InternalEngineObject(Engine* with_engine)
		: SupportsLogging(with_engine->GetLogger()), owner_engine(with_engine)
	{
	}
} // namespace Engine
