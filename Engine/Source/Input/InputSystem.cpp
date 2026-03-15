//
// Created by gorev on 13.03.2026.
//

#include "Input/InputSystem.h"

#include "Tools/Logger.h"

namespace RTGDEngine
{
    InputSystem& InputSystem::Instance()
    {
        static InputSystem instance;
        return instance;
    }

    void InputSystem::Initialize(HWND hwnd, const int width, const int height)
    {
        m_hwnd = hwnd;
        m_manager.SetDisplaySize(width, height);

        m_keyboard = m_manager.CreateDevice<gainput::InputDeviceKeyboard>();
        m_mouse = m_manager.CreateDevice<gainput::InputDeviceMouse>();
        m_mouseRaw = m_manager.CreateDevice<gainput::InputDeviceMouse>(
            gainput::InputDevice::AutoIndex,
            gainput::InputDevice::DV_RAW);

        m_map = std::make_unique<gainput::InputMap>(m_manager);

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
        m_map->MapFloat(ID(A::DeltaX), m_mouseRaw, gainput::MouseAxisX);
        m_map->MapFloat(ID(A::DeltaY), m_mouseRaw, gainput::MouseAxisY);

        LogInfo("InputSystem initialized ({}x{})", width, height);
    }

    void InputSystem::MoveMouseBack()
    {
        if (m_mouseCaptured)
        {
            SetCursorPos(m_savedMousePos.x, m_savedMousePos.y);
        }
    }

    void InputSystem::Update()
    {
        m_manager.Update();

        if (IsPressed(EInputAction::MouseRight))
        {
            LogInfo("Mouse Right Pressed");
            CaptureMouse(true);
        }

        if (IsReleased(EInputAction::MouseRight) ||
            IsPressed(EInputAction::Escape))
        {
            LogInfo("Mouse Right Released");
            CaptureMouse(false);
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

    void InputSystem::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        m_manager.HandleMessage({hwnd, msg, wParam, lParam});

        if (msg == WM_INPUT)
            ProcessRawMouseInput(lParam);
    }

    void InputSystem::ProcessRawMouseInput(LPARAM lParam)
    {
        UINT size = 0;
        GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam),
                        RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));

        if (size == 0)
            return;

        std::vector<BYTE> buffer(size);
        if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam),
                            RID_INPUT, buffer.data(), &size, sizeof(RAWINPUTHEADER)) != size)
            return;

        const auto* raw = reinterpret_cast<const RAWINPUT*>(buffer.data());
        if (raw->header.dwType != RIM_TYPEMOUSE)
            return;

        m_mouseDeltaX += static_cast<float>(raw->data.mouse.lLastX);
        m_mouseDeltaY += static_cast<float>(raw->data.mouse.lLastY);
        //LogInfo("Raw Input: {} {}", m_mouseDeltaX, m_mouseDeltaY);
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

    void InputSystem::CaptureMouse(const bool capture)
    {
        if (m_mouseCaptured == capture)
            return;

        m_mouseCaptured = capture;

        if (capture)
        {
            SetCapture(m_hwnd);
            ShowCursor(FALSE);

            RECT rect;
            GetClientRect(m_hwnd, &rect);
            ClientToScreen(m_hwnd, reinterpret_cast<POINT*>(&rect.left));
            ClientToScreen(m_hwnd, reinterpret_cast<POINT*>(&rect.right));
            ClipCursor(&rect);
        }
        else
        {
            ReleaseCapture();
            ShowCursor(TRUE);
            ClipCursor(nullptr);
        }
    }
}
