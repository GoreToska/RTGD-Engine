//
// Created by ivan on 6/19/26.
//

#include "Input/MouseDevice.h"

namespace RTGDEngine {
    using namespace gainput;

    MouseDevice::MouseDevice(InputManager &manager, DeviceId deviceID, unsigned index, DeviceVariant)
        : InputDevice(manager, deviceID, index == AutoIndex ? manager.GetDeviceCountByType(DT_MOUSE) : index) {
        state_ = manager.GetAllocator().New<InputState>(manager.GetAllocator(), MouseButtonCount_);
        previousState_ = manager.GetAllocator().New<InputState>(manager.GetAllocator(), MouseButtonCount_);
    }

    MouseDevice::~MouseDevice() {
        manager_.GetAllocator().Delete(state_);
        manager_.GetAllocator().Delete(previousState_);
    }

    void MouseDevice::InjectAxis(DeviceButtonId id, float v) {
        m_queue.PushAxis(id, v);
    }

    void MouseDevice::InjectButton(DeviceButtonId id, bool v) {
        m_queue.PushButton(id, v);
    }

    InputDevice::DeviceType MouseDevice::GetType() const {
        return DT_MOUSE;
    }

    InputDevice::DeviceVariant MouseDevice::GetVariant() const {
        return DV_NULL;
    }

    const char *MouseDevice::GetTypeName() const {
        return "RTGD_Mouse";
    }

    bool MouseDevice::IsValidButtonId(DeviceButtonId deviceButton) const {
        return deviceButton < MouseButtonCount_;
    }

    ButtonType MouseDevice::GetButtonType(DeviceButtonId deviceButton) const {
        return deviceButton >= MouseAxisX ? BT_FLOAT : BT_BOOL;
    }

    InputDevice::DeviceState MouseDevice::InternalGetState() const {
        return DS_OK;
    }

    void MouseDevice::InternalUpdate(InputDeltaState *delta) {
        m_queue.Flush(*this, *state_, delta);
    }
} // RTGDEngine
