//
// Created by ivan on 6/19/26.
//

#include "Input/InjectQueue.h"

#include <gainput/GainputInputDeltaState.h>
#include <gainput/GainputHelpers.h>

namespace RTGDEngine {
    void InjectQueue::PushButton(gainput::DeviceButtonId id, bool v) {
        Push({id, v});
    }

    void InjectQueue::Flush(gainput::InputDevice &device, gainput::InputState &state, gainput::InputDeltaState *delta) {
        std::vector<Entry> batch = {};

        {
            std::lock_guard lock(m_mutex);
            batch.swap(m_pending);
        }

        for (auto &[ID, Value]: batch) {
            gainput::HandleButton(device, state, delta, ID, Value);
        }
    }

    void InjectQueue::Push(const Entry &e) {
        std::lock_guard lock(m_mutex);
        m_pending.push_back(e);
    }
} // RTGDEngine
