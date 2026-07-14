//
// Created by ivan on 6/16/26.
//

#pragma once
#if defined(__linux__)
#include "Platform/IPlatformWindow.h"
#include <X11/Xlib.h>
#include <mutex>

namespace RTGDEngine {
    class EmbeddedLinuxWindow : public IPlatformWindow {
    public:
        EmbeddedLinuxWindow(unsigned long XID);

        ~EmbeddedLinuxWindow() override = default;

        bool Create(const WindowDesc &desc) override;

        bool PollEvents() override;

        [[nodiscard]] NativeWindowHandle GetHandle() const override;

        [[nodiscard]] EInputSource GetInputSource() const override;

        void Destroy() override;

        void SetCursorVisible(bool visible) override;

        bool GetMouseDelta(float &dx, float &dy) override;

        void SetRelativeMouseMode(bool relative) override;

        void InjectMouseMove(float dx, float dy) override;

        void WarpCursorToCenter() override;

    private:
        Display *m_display = nullptr; // We own this
        Window m_windowHandle = {}; // We DON'T own this
        XWindowAttributes m_windowAttributes = {};
        std::mutex m_windowMutex = {};
        float m_deltaX = 0.0f;
        float m_deltaY = 0.0f;
    };
}

#endif
