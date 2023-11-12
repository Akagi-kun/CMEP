#pragma once

#include "Object.hpp"
#include "InternalEngineObject.hpp"
#include "PlatformSemantics.hpp"

#include <unordered_map>

namespace Engine
{
    class Engine;

    class CMEP_EXPORT Scene : public InternalEngineObject
    {
    private:
    protected:
        std::unordered_map<std::string, Object*> objects{};
    public:
		Engine* owner_engine;

        Scene();
        ~Scene();

		const std::unordered_map<std::string, Object*>* const GetAllObjects() noexcept;

		void AddObject(std::string name, Object* ptr);
		Object* FindObject(std::string name);
		size_t RemoveObject(std::string name) noexcept;
    };
}