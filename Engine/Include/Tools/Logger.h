//
// Created by gorev on 12.03.2026.
//

#pragma once

#include "Engine/EngineExport.h"
#include <spdlog/spdlog.h>

namespace RTGDEngine
{
    class ENGINE_API Logger
    {
    public:
        static Logger& Instance();

        void Initialize(const std::string& logFile = "Log/Engine.log");

        void SetLevel(spdlog::level::level_enum level);

        spdlog::logger& GetLogger();

    private:
        Logger() = default;

        ~Logger() = default;

        Logger(const Logger&) = delete;

        Logger& operator=(const Logger&) = delete;

        std::unique_ptr<spdlog::logger> m_logger;
        bool m_isInitialized = false;
    };
}

#if defined(_MSC_VER)
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#define LogTrace(...)    RTGDEngine::Logger::Instance().GetLogger().log( \
spdlog::source_loc{__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::trace, __VA_ARGS__)

#define LogInfo(...)     RTGDEngine::Logger::Instance().GetLogger().log( \
spdlog::source_loc{__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::info, __VA_ARGS__)

#define LogWarn(...)     RTGDEngine::Logger::Instance().GetLogger().log( \
spdlog::source_loc{__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::warn, __VA_ARGS__)

#define LogError(...)    RTGDEngine::Logger::Instance().GetLogger().log( \
spdlog::source_loc{__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::err, __VA_ARGS__)

#define LogCritical(...) RTGDEngine::Logger::Instance().GetLogger().log( \
spdlog::source_loc{__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::critical, __VA_ARGS__)
