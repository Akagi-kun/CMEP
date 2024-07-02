#include "InternalEngineObject.hpp"

#include "Engine.hpp"

namespace Engine
{
	InternalEngineObject::InternalEngineObject(Engine* with_engine) : owner_engine(with_engine)
	{
		this->logger = with_engine->GetLogger();
	}
} // namespace Engine
