//
// Created by ivan on 6/26/26.
//
#pragma once
#include <cstdint>
#include <functional>

#include "Tools/RTGDMacros.h"

namespace RTGDEngine {
    struct TimerHandle {
        uint32_t Index = UINT32_MAX;
        uint32_t Generation = 0;

        [[nodiscard]] bool IsValid() const { return Index != UINT32_MAX; }
    };


    class TimerSystem {
        DECLARE_SINGLETON(TimerSystem);

    public:
        TimerHandle SetTimer(std::function<void()> func, float delay, bool loop = false);

        void ClearTimer(TimerHandle &handle);

        bool IsActive(TimerHandle handle) const;

        void Update(float deltaTime);

    private:
        struct Timer {
            std::function<void()> Callback;
            float Remaining = 0.0f;
            float Interval = 0.0f;
            bool Loop = false;
            bool Active = false;
            uint32_t Generation = 0;
        };

        void FreeSlot(uint32_t slot);

        std::vector<Timer> m_timers = {};
        std::vector<uint32_t> m_freeList = {};
        std::vector<std::function<void()> > m_functionsToExecute = {};
    };
} // RTGDEngine
