#pragma once

#include "Logging/Logging.hpp"
//#include "Engine.hpp"
#include "PlatformSemantics.hpp"

#include <optional>

namespace Engine 
{
    class CMEP_EXPORT InternalEngineObject
    {
    protected:
    public:
        std::shared_ptr<Logging::Logger> logger{};
        
        InternalEngineObject() {}
        //~InternalEngineObject() {}

        void UpdateHeldLogger(std::shared_ptr<Logging::Logger> new_logger)
        {
            this->logger = new_logger;
        }
    };
}