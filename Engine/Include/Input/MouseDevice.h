//
// Created by ivan on 6/19/26.
//

#pragma once
#include <gainput/gainput.h>

#include "IInjectable.h"
#include "InjectQueue.h"

namespace RTGDEngine {
    class MouseDevice : public gainput::InputDevice, public IInjectableButton, public IInjectableAxis {
    public:
        MouseDevice(gainput::InputManager &manager, gainput::DeviceId deviceID, unsigned index, DeviceVariant /**/);

        ~MouseDevice() override;

        void InjectAxis(gainput::DeviceButtonId id, float v) override;

        void InjectButton(gainput::DeviceButtonId id, bool v) override;

        DeviceType GetType() const override;

        DeviceVariant GetVariant() const override;

        const char *GetTypeName() const override;

        bool IsValidButtonId(gainput::DeviceButtonId deviceButton) const override;

        gainput::ButtonType GetButtonType(gainput::DeviceButtonId deviceButton) const override;

    protected:
        DeviceState InternalGetState() const override;

        void InternalUpdate(gainput::InputDeltaState *delta) override;

    private:
        InjectQueue m_queue = {};
    };
} // RTGDEngine
