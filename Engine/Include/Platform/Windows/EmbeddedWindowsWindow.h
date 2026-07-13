//
// Created by ivan on 6/22/26.
//

#pragma once
#ifdef _WIN32
#include "Platform/IPlatformWindow.h"
#include <Windows.h>

namespace RTGDEngine {
    class EmbeddedWindowsWindow : public IPlatformWindow {
    public:
        explicit EmbeddedWindowsWindow(HWND hwnd);

        ~EmbeddedWindowsWindow() override = default;

        bool Create(const WindowDesc &desc) override;

        bool PollEvents() override;

        [[nodiscard]] NativeWindowHandle GetHandle() const override;

        [[nodiscard]] EInputSource GetInputSource() const override;

        void Destroy() override;

        void SetCursorVisible(bool visible) override;

        void SetRelativeMouseMode(bool relative) override;

        bool GetMouseDelta(float &dx, float &dy) override;

        void InjectMouseMove(float dx, float dy) override;

        void WarpCursorToCenter() override;

    private:
        HWND m_hwnd = nullptr; // We DON'T own this (the editor owns it)
        HINSTANCE m_hinstance = nullptr;
        float m_deltaX = 0.0f;
        float m_deltaY = 0.0f;
    };
}

#endif
