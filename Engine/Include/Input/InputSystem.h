//
// Created by gorev on 13.03.2026.
//

#pragma once

#include <memory>
#include <gainput/gainput.h>
#include <Windows.h>

#include "EInputAction.h"
#include "Engine/EngineExport.h"

namespace RTGDEngine
{
    class ENGINE_API InputSystem
    {
    public:
        static InputSystem& Instance();

        void Initialize(HWND hwnd, int width, int height);

        void Update();

        void PostUpdate();

        void Resize(int width, int height);

        void HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

        void ProcessRawMouseInput(LPARAM lParam);

        [[nodiscard]] bool IsDown(EInputAction action) const;

        [[nodiscard]] bool IsPressed(EInputAction action) const;

        [[nodiscard]] bool IsReleased(EInputAction action) const;

        [[nodiscard]] float GetAxis(EInputAction action) const;

        [[nodiscard]] bool IsMouseCaptured() const;

        [[nodiscard]] float GetMouseDeltaX() const;

        [[nodiscard]] float GetMouseDeltaY() const;

    private:
        InputSystem() = default;

        void CaptureMouse(bool capture);

        void MoveMouseBack();

        gainput::InputManager m_manager;
        std::unique_ptr<gainput::InputMap> m_map = nullptr;
        gainput::DeviceId m_keyboard = gainput::InvalidDeviceId;
        gainput::DeviceId m_mouse = gainput::InvalidDeviceId;
        gainput::DeviceId m_mouseRaw = gainput::InvalidDeviceId;

        HWND m_hwnd = nullptr;
        bool m_mouseCaptured = false;

        float m_lastMouseX = 0.0f;
        float m_lastMouseY = 0.0f;
        float m_mouseDeltaX = 0.0f;
        float m_mouseDeltaY = 0.0f;
        POINT m_savedMousePos;
    };
}
