#include <cassert>

#include "Logging/Logging.hpp"

#include "test_logging.hpp"
#include "boilerplate.hpp"

int boilerplate_logger_init()
{
	Logging::GlobalLogger = std::make_unique<Logging::Logger>();

    return 0;
}

int boilerplate_logger_release()
{
    Logging::GlobalLogger.release();

    return 0;
}

int test_logging_init_release()
{
    assert(!boilerplate_logger_init());

	Logging::GlobalLogger->AddOutputHandle(Logging::LogLevel::Debug3, stdout, true);

    assert(!boilerplate_logger_release());

    return 0;
}

int test_logging_log()
{
    assert(!boilerplate_logger_init());

	Logging::GlobalLogger->AddOutputHandle(Logging::LogLevel::Debug3, stdout, true);

    Logging::GlobalLogger->StartLog(Logging::LogLevel::Info);
    Logging::GlobalLogger->Log("testing logging %s %u %f 0x%x", "test", 0, 0.0, 0xffff);
    Logging::GlobalLogger->StopLog();

    assert(!boilerplate_logger_release());

    return 0;
}

int test_logging_simplelog()
{
    assert(!boilerplate_logger_init());

	Logging::GlobalLogger->AddOutputHandle(Logging::LogLevel::Debug3, stdout, true);

    Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Info, "testing simplelogging %s %u %f 0x%x", "test", 0, 0.0, 0xffff);

    assert(!boilerplate_logger_release());

    return 0;
}

int test_logging_map_thread_simplelog()
{
    assert(!boilerplate_logger_init());

	Logging::GlobalLogger->AddOutputHandle(Logging::LogLevel::Debug3, stdout, true);

    Logging::GlobalLogger->MapCurrentThreadToName("main");

    Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Info, "testing mapthread simplelogging %s %u %f 0x%x", "test", 0, 0.0, 0xffff);

    assert(!boilerplate_logger_release());

    return 0;
}