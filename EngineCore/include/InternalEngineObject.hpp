#pragma once

#include "Logging/Logging.hpp"
//#include "Engine.hpp"
#include "PlatformSemantics.hpp"

#include <optional>

namespace Engine 
{
    class Engine;

    class CMEP_EXPORT InternalEngineObject
    {
    protected:
    public:
        std::shared_ptr<Logging::Logger> logger{};
        Engine* owner_engine;
        
        InternalEngineObject() {}
        InternalEngineObject(Engine* engine) : owner_engine(engine) {}
        //~InternalEngineObject() {}

        void UpdateHeldLogger(std::shared_ptr<Logging::Logger> new_logger)
        {
            this->logger = new_logger;
        }
    };
}