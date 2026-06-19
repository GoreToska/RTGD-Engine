//
// Created by ivan on 6/16/26.
//

#if defined(__linux__)
#include "Platform/Linux/EmbeddedLinuxWindow.h"

#include "Tools/Logger.h"

namespace RTGDEngine {
    EmbeddedLinuxWindow::EmbeddedLinuxWindow(unsigned long XID)
        : m_windowHandle(XID) {
    }

    bool EmbeddedLinuxWindow::Create(const WindowDesc &desc) {
        m_display = XOpenDisplay(nullptr);

        if (!m_display) {
            LogError("XOpenDisplay failed.");
            return false;
        }

        if (m_windowHandle == 0) {
            LogError("Passed wrong XID in constructor.");
            return false;
        }

        XGetWindowAttributes(m_display, m_windowHandle, &m_windowAttributes);
        SetSize(m_windowAttributes.width, m_windowAttributes.height);
        return true;
    }

    bool EmbeddedLinuxWindow::PollEvents() {
        return true; // we don't need to poll events in embedded window, so just return true
    }

    NativeWindowHandle EmbeddedLinuxWindow::GetHandle() const {
        return {GetWidth(), GetHeight(), m_display, m_windowHandle};
    }

    void EmbeddedLinuxWindow::Destroy() {
        if (m_display) {
            XCloseDisplay(m_display);
            m_display = nullptr;
            m_windowHandle = 0;
        }
    }

    void EmbeddedLinuxWindow::SetCursorVisible(bool visible) {
        // probably should do nothing, because cursor should be controlled by window owner
    }

    void EmbeddedLinuxWindow::SetMouseCapture(bool capture) {
        // don't know what to do for now without editor
    }

    void EmbeddedLinuxWindow::CenterCursor() {
        // don't know what to do for now without editor
    }
}

#endif
