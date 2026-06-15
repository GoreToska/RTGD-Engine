//
// Created by ivan on 6/10/26.
//

#pragma once
#if defined(__linux__)
#include "Platform/IPlatformWindow.h"
#include <X11/Xlib.h>

namespace RTGDEngine {
    class LinuxPlatform : public IPlatformWindow {
    public:
        bool Create(const WindowDesc &desc) override;

        bool PollEvents() override;

        [[nodiscard]] NativeWindowHandle GetHandle() const override;

        void Destroy() override;

        void SetCursorVisible(bool visible) override;

        void SetMouseCapture(bool capture) override;

        void CenterCursor() override;

    private:
        Display *m_display = nullptr;
        Window m_window = 0;
        Atom m_deleteAtom = 0;
        bool m_running = true;
    };
} // RTGDEngine
#endif
