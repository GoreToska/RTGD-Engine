//
// Created by gorev on 12.03.2026.
//

#include "Tools/Logger.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

namespace RTGDEngine
{
    Logger& Logger::Instance()
    {
        static Logger instance;
        return instance;
    }

    void Logger::Initialize(const std::string& logFile)
    {
        std::vector<spdlog::sink_ptr> sinks;

        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        consoleSink->set_pattern("[%H:%M:%S] [%^%l%$] [%s:%#] %v");
        sinks.push_back(consoleSink);

        auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            logFile, 1024 * 1024 * 5, 3);
        fileSink->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] [%s:%#] %v");
        sinks.push_back(fileSink);

        m_logger = std::make_unique<spdlog::logger>("RTGD", sinks.begin(), sinks.end());
        m_logger->set_level(spdlog::level::trace);
        m_logger->flush_on(spdlog::level::warn);

        m_isInitialized = true;
    }

    void Logger::SetLevel(const spdlog::level::level_enum level)
    {
        assert(m_isInitialized && "Logger is not initialized");
        m_logger->set_level(level);
    }

    spdlog::logger& Logger::GetLogger()
    {
        assert(m_isInitialized && "Logger is not initialized");
        return *m_logger;
    }
}
