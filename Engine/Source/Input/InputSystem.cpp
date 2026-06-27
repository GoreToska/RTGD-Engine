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

namespace RTGDEngine
{
    void InputSystem::CreateInputMapping()
    {
        m_map = std::make_unique<InputMap>(m_manager);

        using A = EInputAction;
        m_map->MapBool(ID(A::MoveForward), m_keyboard, gainput::KeyW);
        m_map->MapBool(ID(A::MoveBackward), m_keyboard, gainput::KeyS);
        m_map->MapBool(ID(A::MoveLeft), m_keyboard, gainput::KeyA);
        m_map->MapBool(ID(A::MoveRight), m_keyboard, gainput::KeyD);
        m_map->MapBool(ID(A::MoveUp), m_keyboard, gainput::KeyE);
        m_map->MapBool(ID(A::MoveDown), m_keyboard, gainput::KeyQ);
        m_map->MapBool(ID(A::SpeedBoost), m_keyboard, gainput::KeyShiftL);
        m_map->MapBool(ID(A::Escape), m_keyboard, gainput::KeyEscape);

        m_map->MapBool(ID(A::MouseRight), m_mouse, gainput::MouseButtonRight);
        m_map->MapFloat(ID(A::LookX), m_mouse, gainput::MouseAxisX);
        m_map->MapFloat(ID(A::LookY), m_mouse, gainput::MouseAxisY);
    }

    // TODO: need to initialize once and just switch focus of windows
    void InputSystem::InitializeInputForWindow(IPlatformWindow* handle)
    {
        m_manager.SetDisplaySize(handle->GetHandle().width, handle->GetHandle().height);

        CreateKeyboardDevice();
        CreateMouseDevice(handle->GetInputSource());

        CreateInputMapping();

        LogInfo("InputSystem initialized ({}x{})", handle->GetHandle().width, handle->GetHandle().height);
    }

    void InputSystem::AddWindowHandle(IPlatformWindow* window)
    {
        // TODO: add to vector of current windows

        m_platformWindow = window;

        window->OnNativeEvent = [this](const NativeWindowEvent& event)
        {
            this->HandleNativeEvent(event);
        };

        InitializeInputForWindow(window);
    }

    void InputSystem::HandleNativeEvent(const NativeWindowEvent& event)
    {
#ifdef _WIN32
        switch (event.Msg.message)
        {
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYUP:
            {
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

    void InputSystem::MoveMouseBack()
    {
        if (m_mouseCaptured)
        {
            m_platformWindow->CenterCursor();
        }
    }

    void InputSystem::CalculateMouseDelta()
    {
        if (m_ignoreNextDelta)
        {
            m_mouseDeltaX = 0.0f;
            m_mouseDeltaY = 0.0f;
            m_ignoreNextDelta = false;
        }
        else
        {
            // if the mouse is captured, it is always at the center of the screen, so to calc delta we need to subtract half screen from current mouse pos
            m_mouseDeltaX = m_currentMouseX - 0.5f * static_cast<float>(m_platformWindow->GetWidth());
            m_mouseDeltaY = m_currentMouseY - 0.5f * static_cast<float>(m_platformWindow->GetHeight());
        }
    }

    void InputSystem::Update()
    {
        m_manager.Update();

        if (IsPressed(EInputAction::MouseRight))
        {
            CaptureMouse(true);
        }

        if (IsReleased(EInputAction::MouseRight) ||
            IsPressed(EInputAction::Escape))
        {
            CaptureMouse(false);
        }

        if (!m_platformWindow)
            return;

        // Actual mouse position in pixels
        m_currentMouseX = m_map->GetFloat(ID(EInputAction::LookX)) * static_cast<float>(m_platformWindow->GetWidth());
        m_currentMouseY = m_map->GetFloat(ID(EInputAction::LookY)) * static_cast<float>(m_platformWindow->GetHeight());

        if (IsMouseCaptured())
        {
            CalculateMouseDelta();
            m_platformWindow->CenterCursor();
        }
    }

    void InputSystem::PostUpdate()
    {
        m_mouseDeltaX = 0;
        m_mouseDeltaY = 0;
    }

    void InputSystem::Resize(const int width, const int height)
    {
        m_manager.SetDisplaySize(width, height);
    }

    bool InputSystem::IsDown(const EInputAction action) const
    {
        return m_map->GetBool(ID(action));
    }

    bool InputSystem::IsPressed(const EInputAction action) const
    {
        return m_map->GetBoolIsNew(ID(action));
    }

    bool InputSystem::IsReleased(const EInputAction action) const
    {
        return m_map->GetBoolWasDown(ID(action));
    }

    float InputSystem::GetAxis(EInputAction action) const
    {
        return m_map->GetFloat(ID(action));
    }

    bool InputSystem::IsMouseCaptured() const
    {
        return m_mouseCaptured;
    }

    float InputSystem::GetMouseDeltaX() const
    {
        return m_mouseDeltaX;
    }

    float InputSystem::GetMouseDeltaY() const
    {
        return m_mouseDeltaY;
    }

    void InputSystem::InjectKey(gainput::Key key, bool down) const
    {
        if (m_injectKeyboard)
            m_injectKeyboard->InjectButton(key, down);
    }

    void InputSystem::InjectMouseButton(gainput::MouseButton button, bool down) const
    {
        if (m_injectMouseButton)
            m_injectMouseButton->InjectButton(button, down);
    }

    void InputSystem::InjectMousePosition(float normX, float normY) const
    {
        if (!m_injectMouseAxis)
            return;

        m_injectMouseAxis->InjectAxis(gainput::MouseAxisX, normX);
        m_injectMouseAxis->InjectAxis(gainput::MouseAxisY, normY);
    }

    void InputSystem::CreateKeyboardDevice()
    {
        m_keyboard = m_manager.CreateDevice<KeyboardDevice>();
        m_injectKeyboard = dynamic_cast<IInjectableButton*>(m_manager.GetDevice(m_keyboard));
    }

    void InputSystem::CreateMouseDevice(EInputSource source)
    {
        switch (source)
        {
            case EInputSource::NativeEvents:
                m_mouse = m_manager.CreateDevice<gainput::InputDeviceMouse>();
                break;
            case EInputSource::Injected:
                m_mouse = m_manager.CreateDevice<MouseDevice>();
                m_injectMouseButton = dynamic_cast<IInjectableButton*>(m_manager.GetDevice(m_mouse));
                m_injectMouseAxis = dynamic_cast<IInjectableAxis*>(m_manager.GetDevice(m_mouse));
                break;
        }
    }

    void InputSystem::CaptureMouse(const bool capture)
    {
        if (m_mouseCaptured == capture)
            return;

        m_mouseCaptured = capture;

        if (!m_platformWindow)
            return;

        if (capture)
        {
            m_platformWindow->SetCursorVisible(false);
            m_platformWindow->SetMouseCapture(true);
            m_platformWindow->CenterCursor();
            m_ignoreNextDelta = true;
        }
        else
        {
            m_platformWindow->SetCursorVisible(true);
            m_platformWindow->SetMouseCapture(false);
        }
    }
}
