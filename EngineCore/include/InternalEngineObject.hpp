#pragma once

#include "Logging/Logging.hpp"

namespace Engine 
{
    class InternalEngineObject
    {
    protected:
        std::shared_ptr<Logging::Logger> logger{};
    public:
        InternalEngineObject() {}
        //~InternalEngineObject() {}

        int UpdateHeldLogger(std::shared_ptr<Logging::Logger> new_logger)
        {
            this->logger = new_logger;
        }
    };
}