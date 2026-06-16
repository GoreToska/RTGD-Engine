//
// Created by gorev on 13.03.2026.
//

#pragma once

#include <memory>
#include <gainput/gainput.h>

#include "EInputAction.h"
#include "Engine/EngineExport.h"
#include "Platform/WindowEvent.h"
#include "Tools/RTGDMacros.h"

namespace RTGDEngine {
    class IPlatformWindow;

    class ENGINE_API InputSystem {
        DECLARE_SINGLETON(InputSystem);

    public:
        void AddWindowHandle(IPlatformWindow *window);

        void HandleNativeEvent(const NativeWindowEvent &event);

        void Update();

        void PostUpdate();

        void Resize(int width, int height);

        [[nodiscard]] bool IsDown(EInputAction action) const;

        [[nodiscard]] bool IsPressed(EInputAction action) const;

        [[nodiscard]] bool IsReleased(EInputAction action) const;

        [[nodiscard]] float GetAxis(EInputAction action) const;

        [[nodiscard]] bool IsMouseCaptured() const;

        [[nodiscard]] float GetMouseDeltaX() const;

        [[nodiscard]] float GetMouseDeltaY() const;

    private:
        void InitializeInputForWindow(IPlatformWindow *handle);

        void CaptureMouse(bool capture);

        void MoveMouseBack();

        void CalculateMouseDelta();

        gainput::InputManager m_manager;
        std::unique_ptr<gainput::InputMap> m_map = nullptr;
        gainput::DeviceId m_keyboard = gainput::InvalidDeviceId;
        gainput::DeviceId m_mouse = gainput::InvalidDeviceId;

        IPlatformWindow *m_platformWindow = nullptr;

        bool m_mouseCaptured = false;
        bool m_ignoreNextDelta = false;

        float m_currentMouseX = 0.0f;
        float m_currentMouseY = 0.0f;
        float m_mouseDeltaX = 0.0f;
        float m_mouseDeltaY = 0.0f;

        // TODO: add vector of current windows
    };
}
