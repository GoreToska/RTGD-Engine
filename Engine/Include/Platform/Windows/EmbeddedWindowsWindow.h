//
// Created by ivan on 6/22/26.
//

#pragma once
#ifdef _WIN32
#include "Platform/IPlatformWindow.h"
#include <Windows.h>
#include <commctrl.h>
#include <mutex>

namespace RTGDEngine
{
    class EmbeddedWindowsWindow : public IPlatformWindow
    {
    public:
        explicit EmbeddedWindowsWindow(HWND hwnd);

        ~EmbeddedWindowsWindow() override = default;

        bool Create(const WindowDesc& desc) override;

        bool PollEvents() override;

        [[nodiscard]] NativeWindowHandle GetHandle() const override;

        [[nodiscard]] EInputSource GetInputSource() const override;

        void Destroy() override;

        void SetCursorVisible(bool visible) override;

        void SetRelativeMouseMode(bool relative) override;

        bool GetMouseDelta(float& dx, float& dy) override;

        void InjectMouseMove(float dx, float dy) override;

        void WarpCursorToCenter() override;

    private:
        static LRESULT CALLBACK SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass,
                                             DWORD_PTR dwRefData);

        HWND m_hwnd = nullptr; // We DON'T own this (the editor owns it)
        HINSTANCE m_hinstance = nullptr;
        std::mutex m_windowMutex = {};
        float m_deltaX = 0.0f;
        float m_deltaY = 0.0f;
        bool m_subclassed = false;
    };
}

#endif
