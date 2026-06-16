//
// Created by ivan on 6/16/26.
//

#pragma once
#if defined(__linux__)
#include "Platform/IPlatformWindow.h"
#include <X11/Xlib.h>

namespace RTGDEngine {
    class EmbeddedLinuxWindow : public IPlatformWindow {
    public:
        EmbeddedLinuxWindow(unsigned long XID, int width, int height);

        ~EmbeddedLinuxWindow() override = default;

        bool Create(const WindowDesc &desc) override;

        bool PollEvents() override;

        [[nodiscard]] NativeWindowHandle GetHandle() const override;

        void Destroy() override;

        void SetCursorVisible(bool visible) override;

        void SetMouseCapture(bool capture) override;

        void CenterCursor() override;

    private:
        Display *m_display = nullptr; // We own this
        Window m_windowHandle = {}; // We DON'T own this
        XWindowAttributes m_windowAttributes = {};
    };
}

#endif
