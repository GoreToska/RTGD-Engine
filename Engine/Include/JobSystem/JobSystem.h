//
// Created by gorev on 16.03.2026.
//

#pragma once

#include "Engine/EngineExport.h"
#include <functional>
#include <mutex>

#include "TaskScheduler.h"
#include "Tools/RTGDMacros.h"


using JobHandle = enki::TaskSet;
using JobScheduler = enki::TaskScheduler;

namespace RTGDEngine
{
    class ENGINE_API JobSystem
    {
        DECLARE_SINGLETON(JobSystem);

    public:
        void Initialize();

        void Shutdown();

        void Flush(uint32_t maxToRemove = UINT32_MAX);

        void Submit(std::function<void()> job);

        std::shared_ptr<JobHandle> SubmitAndTrack(std::function<void()> job);

        void Wait(std::shared_ptr<JobHandle> jobHandle);

        JobScheduler& GetScheduler();

    private:
        JobScheduler m_scheduler;
        std::mutex m_tasksMutex;
        std::vector<std::shared_ptr<JobHandle>> m_activeTasks = {};
    };
} // RTGDEngine
