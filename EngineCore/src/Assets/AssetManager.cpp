#include "Assets/AssetManager.hpp"

#include "Logging/Logging.hpp"

#include <cassert>
#include <memory>

namespace Engine
{
#pragma region Public

	AssetManager::AssetManager(const Logging::SupportsLogging::logger_t& with_logger)
		: Logging::SupportsLogging(with_logger)
	{}

	AssetManager::~AssetManager()
	{
		this->logger->simpleLog<decltype(this)>(
			Logging::LogLevel::Debug,
			"Destructor called"
		);
	}

	void AssetManager::setSceneRepository(AssetRepository* with_repository)
	{
		repository = with_repository;
	}

#pragma endregion

} // namespace Engine
