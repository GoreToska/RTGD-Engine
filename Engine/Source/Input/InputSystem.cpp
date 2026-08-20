//
// Created by gorev on 13.03.2026.
//

#include "Input/InputSystem.h"

#include "Input/KeyboardDevice.h"
#include "Input/MouseDevice.h"
#include "Input/PhysicalKeyMap.h"
#include "Platform/IPlatformWindow.h"
#include "Tools/Logger.h"

#ifdef _WIN32
#include <Windows.h>
#include <vector>
#elif defined(__linux__)
#include <X11/Xlib.h>
#endif

namespace RTGDEngine {
    void InputSystem::CreateInputMapping() {
        m_map = std::make_unique<InputMap>(m_manager);

        using A = EInputAction;
        m_map->MapBool(ID(A::MoveForward), m_keyboard, KeyW);
        m_map->MapBool(ID(A::MoveBackward), m_keyboard, KeyS);
        m_map->MapBool(ID(A::MoveLeft), m_keyboard, KeyA);
        m_map->MapBool(ID(A::MoveRight), m_keyboard, KeyD);
        m_map->MapBool(ID(A::MoveUp), m_keyboard, KeyE);
        m_map->MapBool(ID(A::MoveDown), m_keyboard, KeyQ);
        m_map->MapBool(ID(A::SpeedBoost), m_keyboard, KeyShiftL);
        m_map->MapBool(ID(A::Escape), m_keyboard, KeyEscape);

        m_map->MapBool(ID(A::CtrlLeft), m_keyboard, KeyCtrlL);
        m_map->MapBool(ID(A::ReloadGameModule), m_keyboard, KeyR);
        m_map->MapBool(ID(A::TogglePlayMode), m_keyboard, KeyP);

        m_map->MapBool(ID(A::MouseRight), m_mouse, MouseButtonRight);
        m_map->MapFloat(ID(A::LookX), m_mouse, MouseAxisX);
        m_map->MapFloat(ID(A::LookY), m_mouse, MouseAxisY);
    }

    // TODO: need to initialize once and just switch focus of windows
    void InputSystem::InitializeInputForWindow(IPlatformWindow *handle) {
        m_manager.SetDisplaySize(handle->GetHandle().width, handle->GetHandle().height);

        CreateKeyboardDevice();
        CreateMouseDevice(handle->GetInputSource());

        CreateInputMapping();

        LogInfo("InputSystem initialized ({}x{})", handle->GetHandle().width, handle->GetHandle().height);
    }

    void InputSystem::AddWindowHandle(IPlatformWindow *window) {
        // TODO: add to vector of current windows

        m_platformWindow = window;

        window->OnNativeEvent = [this](const NativeWindowEvent &event) {
            this->HandleNativeEvent(event);
        };

        InitializeInputForWindow(window);
    }

    void InputSystem::HandleNativeEvent(const NativeWindowEvent &event) {
#ifdef _WIN32
        switch (event.Msg.message) {
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYUP: {
                const bool down = (event.Msg.message == WM_KEYDOWN || event.Msg.message == WM_SYSKEYDOWN);
                const unsigned sc = (event.Msg.lParam >> 16) & 0xFF;
                const bool ext = (event.Msg.lParam >> 24) & 1;
                Key key;
                if (PhysicalToGainput(sc, ext, key))
                    InjectKey(key, down);

                break;
            }
        }

        m_manager.HandleMessage(event.Msg);

#elif defined(__linux__)
        auto *xe = static_cast<XEvent *>(event.XEvent);
        if (xe->type == KeyPress || xe->type == KeyRelease) {
            const bool down = (xe->type == KeyPress);
            const int evdev = static_cast<int>(xe->xkey.keycode) - 8;

            if (Key key; PhysicalToGainput(evdev, key))
                InjectKey(key, down);

            return;
        }
        m_manager.HandleEvent(*xe);
#endif
    }

    void InputSystem::Update() {
        m_manager.Update();

        if (IsPressed(EInputAction::MouseRight))
            SetRelativeMouseMode(true);
        if (IsReleased(EInputAction::MouseRight) || IsPressed(EInputAction::Escape))
            SetRelativeMouseMode(false);

        if (!m_platformWindow || !m_mouseCaptured)
            return;

        float dx = 0.0f, dy = 0.0f;
        if (m_platformWindow->GetMouseDelta(dx, dy)) {
            m_mouseDeltaX = dx;
            m_mouseDeltaY = dy;
        }
    }

    void InputSystem::PostUpdate() {
        m_mouseDeltaX = 0;
        m_mouseDeltaY = 0;
    }

    void InputSystem::Resize(const int width, const int height) {
        m_manager.SetDisplaySize(width, height);
    }

    bool InputSystem::IsDown(const EInputAction action) const {
        return m_map->GetBool(ID(action));
    }

    bool InputSystem::IsDown(ActionID action) const {
        return m_map->GetBool(action);
    }

    bool InputSystem::IsPressed(ActionID action) const {
        return m_map->GetBoolIsNew(action);
    }

    bool InputSystem::IsReleased(ActionID action) const {
        return m_map->GetBoolWasDown(action);
    }

    float InputSystem::GetAxis(ActionID action) const {
        return m_map->GetFloat(action);
    }

    ActionID InputSystem::RegisterAction(const std::string &name) {
        if (auto it = m_customActions.find(name); it != m_customActions.end())
            return it->second;

        ActionID id = m_nextActionID++;
        m_customActions.emplace(name, id);
        return id;
    }

    void InputSystem::BindKey(ActionID action, gainput::Key key) const {
        m_map->MapBool(action, m_keyboard, key);
    }

    void InputSystem::BindMouseButton(ActionID action, gainput::MouseButton button) const {
        m_map->MapBool(action, m_mouse, button);
    }

    bool InputSystem::IsPressed(const EInputAction action) const {
        return m_map->GetBoolIsNew(ID(action));
    }

    bool InputSystem::IsReleased(const EInputAction action) const {
        return m_map->GetBoolWasDown(ID(action));
    }

    float InputSystem::GetAxis(EInputAction action) const {
        return m_map->GetFloat(ID(action));
    }

    bool InputSystem::IsMouseCaptured() const {
        return m_mouseCaptured;
    }

    float InputSystem::GetMouseDeltaX() const {
        return m_mouseDeltaX;
    }

    float InputSystem::GetMouseDeltaY() const {
        return m_mouseDeltaY;
    }

    void InputSystem::InjectKey(Key key, bool down) const {
        if (m_injectKeyboard)
            m_injectKeyboard->InjectButton(key, down);
    }

    void InputSystem::InjectMouseButton(MouseButton button, bool down) const {
        if (m_injectMouseButton)
            m_injectMouseButton->InjectButton(button, down);
    }

    void InputSystem::SetRelativeMouseMode(bool relative) {
        if (!m_platformWindow || m_mouseCaptured == relative)
            return;

        m_mouseCaptured = relative;
        m_platformWindow->SetRelativeMouseMode(relative);
    }

    void InputSystem::InjectMouseMove(float dx, float dy) const {
        if (m_platformWindow)
            m_platformWindow->InjectMouseMove(dx, dy);
    }

    void InputSystem::WarpCursorToCenter() const {
        if (m_platformWindow)
            m_platformWindow->WarpCursorToCenter();
    }

    void InputSystem::SetCursorVisible(bool visible) const {
        if (m_platformWindow)
            m_platformWindow->SetCursorVisible(visible);
    }

    void InputSystem::CreateKeyboardDevice() {
        m_keyboard = m_manager.CreateDevice<KeyboardDevice>();
        m_injectKeyboard = dynamic_cast<IInjectableButton *>(m_manager.GetDevice(m_keyboard));
    }

    void InputSystem::CreateMouseDevice(EInputSource source) {
        switch (source) {
            case EInputSource::NativeEvents:
                m_mouse = m_manager.CreateDevice<InputDeviceMouse>();
                break;
            case EInputSource::Injected:
                m_mouse = m_manager.CreateDevice<MouseDevice>();
                m_injectMouseButton = dynamic_cast<IInjectableButton *>(m_manager.GetDevice(m_mouse));
                break;
        }
    }
}
