//
// Created by ivan on 6/19/26.
//

#pragma once
#include <mutex>
#include <vector>
#include <gainput/gainput.h>
#include <gainput/GainputHelpers.h>

#include "IInjectable.h"
#include "InjectQueue.h"

namespace RTGDEngine {
    using namespace gainput;

    class KeyboardDevice : public InputDevice, public IInjectableButton {
    public:
        KeyboardDevice(InputManager &manager, DeviceId deviceID, unsigned index, DeviceVariant /**/);

        ~KeyboardDevice() override;

        [[nodiscard]] DeviceType GetType() const override;

        [[nodiscard]] DeviceVariant GetVariant() const override;

        [[nodiscard]] const char *GetTypeName() const override;

        [[nodiscard]] bool IsValidButtonId(DeviceButtonId id) const override;

        [[nodiscard]] ButtonType GetButtonType(DeviceButtonId deviceButton) const override;

        void InjectButton(DeviceButtonId key, bool down) override;

    protected:
        void InternalUpdate(InputDeltaState *delta) override;

        [[nodiscard]] DeviceState InternalGetState() const override;

    private:
        struct KeyEvent {
            DeviceButtonId buttonId;
            bool down;
        };

        std::mutex m_mutex = {};
        InjectQueue m_queue = {};
    };
} // RTGDEngine
