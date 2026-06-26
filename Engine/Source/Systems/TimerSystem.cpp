//
// Created by ivan on 6/26/26.
//

#include "Systems/TimerSystem.h"

// TODO: add mutex to use this in job system
namespace RTGDEngine {
    TimerHandle TimerSystem::SetTimer(std::function<void()> func, float delay, bool loop) {
        uint32_t i;
        if (!m_freeList.empty()) {
            i = m_freeList.back();
            m_freeList.pop_back();
        } else {
            i = m_timers.size();
            m_timers.push_back({});
        }

        Timer &timer = m_timers[i];
        timer.Callback = std::move(func);
        timer.Remaining = delay;
        timer.Interval = delay;
        timer.Loop = loop;
        timer.Active = true;
        return {i, timer.Generation};
    }

    void TimerSystem::ClearTimer(TimerHandle &handle) {
        if (!IsActive(handle)) return;
        FreeSlot(handle.Index);
        handle = {};
    }

    bool TimerSystem::IsActive(TimerHandle handle) const {
        return handle.IsValid() && handle.Index < m_timers.size() && m_timers[handle.Index].Generation == handle.
               Generation && m_timers[handle.Index].Active;
    }

    void TimerSystem::Update(float deltaTime) {
        m_functionsToExecute.clear();

        for (uint32_t i = 0; i < m_timers.size(); ++i) {
            Timer &timer = m_timers[i];

            if (!timer.Active) continue;

            timer.Remaining -= deltaTime;
            if (timer.Remaining > 0.0f) continue;

            m_functionsToExecute.push_back(timer.Callback);
            if (timer.Loop)
                timer.Remaining += timer.Interval;
            else
                FreeSlot(i);
        }

        for (auto &func: m_functionsToExecute) {
            if (func) func();
        }
    }

    void TimerSystem::FreeSlot(uint32_t slot) {
        m_timers[slot].Active = false;
        m_timers[slot].Callback = nullptr;
        ++m_timers[slot].Generation;
        m_freeList.push_back(slot);
    }
} // RTGDEngine
