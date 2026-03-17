//
// Created by gorev on 16.03.2026.
//

#include "JobSystem/JobSystem.h"

namespace RTGDEngine
{
    void JobSystem::Initialize()
    {
        m_scheduler.Initialize();
        m_activeTasks.reserve(32);
    }

    void JobSystem::Shutdown()
    {
        m_scheduler.WaitforAllAndShutdown();
    }

    void JobSystem::Flush(const uint32_t maxToRemove)
    {
        std::lock_guard lock(m_tasksMutex);
        uint32_t removed = 0;
        std::erase_if(m_activeTasks,
                      [&](const std::shared_ptr<enki::TaskSet>& t)
                      {
                          if (removed >= maxToRemove)
                              return false;
                          if (t->GetIsComplete())
                          {
                              removed++;
                              return true;
                          }
                          return false;
                      });
    }

    void JobSystem::Submit(std::function<void()> job)
    {
        auto handle = std::make_shared<JobHandle>(
            1, [job = std::move(job)](enki::TaskSetPartition, uint32_t)
            {
                job();
            }); {
            std::lock_guard lock(m_tasksMutex);
            m_activeTasks.push_back(handle);
        }

        m_scheduler.AddTaskSetToPipe(handle.get());
    }

    std::shared_ptr<JobHandle> JobSystem::SubmitAndTrack(std::function<void()> job)
    {
        auto handle = std::make_shared<JobHandle>(
            1, [job = std::move(job)](enki::TaskSetPartition, uint32_t)
            {
                job();
            }); {
            std::lock_guard lock(m_tasksMutex);
            m_activeTasks.push_back(handle);
        }

        m_scheduler.AddTaskSetToPipe(handle.get());
        return handle;
    }

    void JobSystem::Wait(std::shared_ptr<JobHandle> jobHandle)
    {
        if (jobHandle)
            m_scheduler.WaitforTask(jobHandle.get());
    }

    JobScheduler& JobSystem::GetScheduler()
    {
        return m_scheduler;
    }
} // RTGDEngine
