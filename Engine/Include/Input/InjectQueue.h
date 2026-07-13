//
// Created by ivan on 6/19/26.
//

#pragma once
#include <mutex>
#include <vector>

#include "gainput/gainput.h"

namespace RTGDEngine {
    class InjectQueue {
    public:
        void PushButton(gainput::DeviceButtonId id, bool v);

        void Flush(gainput::InputDevice &device, gainput::InputState &state, gainput::InputDeltaState *delta);

    private:
        struct Entry {
            gainput::DeviceButtonId ID;
            bool Value;
        };

        void Push(const Entry &e);

        std::mutex m_mutex = {};
        std::vector<Entry> m_pending = {};
    };
} // RTGDEngine
