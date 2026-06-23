//
// Created by ivan on 6/19/26.
//

#include "Input/InjectQueue.h"

#include <gainput/GainputInputDeltaState.h>
#include <gainput/GainputHelpers.h>

namespace RTGDEngine {
    void InjectQueue::PushButton(gainput::DeviceButtonId id, bool v) {
        Push({id, false, v, 0.0f});
    }

    void InjectQueue::PushAxis(gainput::DeviceButtonId id, float v) {
        Push({id, true, false, v});
    }

    void InjectQueue::Flush(gainput::InputDevice &device, gainput::InputState &state, gainput::InputDeltaState *delta) {
        std::vector<Entry> batch = {};

        {
            std::lock_guard lock(m_mutex);
            batch.swap(m_pending);
        }

        for (auto &[ID, IsAxis, BValue, FValue]: batch) {
            if (IsAxis) gainput::HandleAxis(device, state, delta, ID, FValue);
            else gainput::HandleButton(device, state, delta, ID, BValue);
        }
    }

    void InjectQueue::Push(const Entry &e) {
        std::lock_guard lock(m_mutex);
        m_pending.push_back(e);
    }
} // RTGDEngine
