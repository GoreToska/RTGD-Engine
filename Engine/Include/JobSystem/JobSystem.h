//
// Created by gorev on 16.03.2026.
//

#pragma once

#include "Engine/EngineExport.h"
#include <functional>
#include <memory>
#include <mutex>

#include "TaskScheduler.h"
#include "Tools/RTGDMacros.h"


using JobHandle = enki::TaskSet;
using JobScheduler = enki::TaskScheduler;

namespace RTGDEngine {
    namespace Detail {
        template<typename Fn>
        class ParallelForTask final : public enki::ITaskSet {
        public:
            ParallelForTask(uint32_t setSize, uint32_t minRange, Fn &fn)
                : ITaskSet(setSize, minRange), m_fn(fn) {
            }

            void ExecuteRange(enki::TaskSetPartition range_, uint32_t threadnum_) override {
                m_fn(range_.start, range_.end, threadnum_);
            }

        private:
            Fn &m_fn;
        };
    }

    class ENGINE_API JobSystem {
        DECLARE_SINGLETON(JobSystem);

    public:
        void Initialize();

        void Shutdown();

        void Flush(uint32_t maxToRemove = UINT32_MAX);

        void Submit(std::function<void()> job);

        std::shared_ptr<JobHandle> SubmitAndTrack(std::function<void()> job);

        void Wait(std::shared_ptr<JobHandle> jobHandle);

        JobScheduler &GetScheduler();

        template<typename Fn>
        void ParallelFor(uint32_t count, uint32_t grainSize, Fn &&fn) {
            if (count == 0)
                return;

            if (grainSize == 0)
                grainSize = 1;

            if (count <= grainSize || m_scheduler.GetNumTaskThreads() <= 1) {
                fn(0u, count, m_scheduler.GetThreadNum());
                return;
            }

            Detail::ParallelForTask<std::remove_reference_t<Fn> > task(count, grainSize, fn);
            m_scheduler.AddTaskSetToPipe(&task);
            m_scheduler.WaitforTask(&task);
        }

    private:
        JobScheduler m_scheduler = {};
        std::mutex m_tasksMutex = {};
        std::vector<std::shared_ptr<JobHandle> > m_activeTasks = {};
    };
} // RTGDEngine
