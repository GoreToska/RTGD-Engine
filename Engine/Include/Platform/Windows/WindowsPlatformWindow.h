//
// Created by ivan on 6/10/26.
//

#pragma once
#ifdef _WIN32
#include "Platform/IPlatformWindow.h"
#include <Windows.h>

namespace RTGDEngine {
    class WindowsPlatformWindow : public IPlatformWindow {
    public:
        bool Create(const WindowDesc &desc) override;

        bool PollEvents() override;

        void Destroy() override;

        NativeWindowHandle GetHandle() const override;

        [[nodiscard]] EInputSource GetInputSource() const override;

        void SetCursorVisible(bool visible) override;

        void SetRelativeMouseMode(bool relative) override;

        bool GetMouseDelta(float &dx, float &dy) override;

    private:
        static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

        HWND m_hwnd = nullptr;
        HINSTANCE m_hinstance = nullptr;
        bool m_running = true;
        float m_deltaX = 0.0f;
        float m_deltaY = 0.0f;
    };
} // RTGDEngine

#endif
