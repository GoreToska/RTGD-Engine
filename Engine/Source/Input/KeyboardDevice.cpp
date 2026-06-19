//
// Created by ivan on 6/19/26.
//

#include "Input/KeyboardDevice.h"

namespace RTGDEngine {
    KeyboardDevice::KeyboardDevice(InputManager &manager, DeviceId deviceID, unsigned index, DeviceVariant)
        : InputDevice(manager, deviceID, index == AutoIndex ? manager.GetDeviceCountByType(DT_KEYBOARD) : index) {
        state_ = manager.GetAllocator().New<InputState>(manager.GetAllocator(), KeyCount_);
        previousState_ = manager.GetAllocator().New<InputState>(manager.GetAllocator(), KeyCount_);
    }

    KeyboardDevice::~KeyboardDevice() {
        manager_.GetAllocator().Delete(state_);
        manager_.GetAllocator().Delete(previousState_);
    }

    InputDevice::DeviceType KeyboardDevice::GetType() const {
        return DT_KEYBOARD;
    }

    InputDevice::DeviceVariant KeyboardDevice::GetVariant() const {
        return DV_NULL;
    }

    const char *KeyboardDevice::GetTypeName() const {
        return "RTGD_Keyboard";
    }

    bool KeyboardDevice::IsValidButtonId(DeviceButtonId id) const {
        return id < KeyCount_;
    }

    ButtonType KeyboardDevice::GetButtonType(DeviceButtonId deviceButton) const {
        return BT_BOOL;
    }

    void KeyboardDevice::InjectButton(DeviceButtonId key, bool down) {
        m_queue.PushButton(key, down);
    }

    void KeyboardDevice::InternalUpdate(InputDeltaState *delta) {
        m_queue.Flush(*this, *state_, delta);
    }

    InputDevice::DeviceState KeyboardDevice::InternalGetState() const {
        // for now always return ok, later mb we will validate something here
        return DS_OK;
    }
} // RTGDEngine
