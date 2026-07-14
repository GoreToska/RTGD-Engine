//
// Created by ivan on 6/16/26.
//

#if defined(__linux__)
#include "Platform/Linux/EmbeddedLinuxWindow.h"

#include <X11/extensions/Xfixes.h>

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
        XEvent ev;
        while (XPending(m_display) > 0)
            XNextEvent(m_display, &ev);
        return true;
    }

    bool EmbeddedLinuxWindow::GetMouseDelta(float &dx, float &dy) {
        std::lock_guard<std::mutex> lock(m_windowMutex);
        dx = m_deltaX;
        dy = m_deltaY;
        m_deltaX = 0.0f;
        m_deltaY = 0.0f;
        return true;
    }

    void EmbeddedLinuxWindow::SetRelativeMouseMode(bool relative) {
        if (relative) {
            std::lock_guard<std::mutex> lock(m_windowMutex);
            m_deltaX = 0.0f;
            m_deltaY = 0.0f;
        }
    }

    void EmbeddedLinuxWindow::InjectMouseMove(float dx, float dy) {
        std::lock_guard<std::mutex> lock(m_windowMutex);
        m_deltaX += dx;
        m_deltaY += dy;
    }

    void EmbeddedLinuxWindow::WarpCursorToCenter() {
        XWarpPointer(m_display, None, m_windowHandle, 0, 0, 0, 0, GetWidth() / 2, GetHeight() / 2);
        XFlush(m_display);
    }

    NativeWindowHandle EmbeddedLinuxWindow::GetHandle() const {
        return {GetWidth(), GetHeight(), m_display, m_windowHandle};
    }

    EInputSource EmbeddedLinuxWindow::GetInputSource() const {
        return EInputSource::Injected;
    }

    void EmbeddedLinuxWindow::Destroy() {
        if (m_display) {
            XCloseDisplay(m_display);
            m_display = nullptr;
            m_windowHandle = 0;
        }
    }

    void EmbeddedLinuxWindow::SetCursorVisible(bool visible) {
        if (visible) XFixesShowCursor(m_display, m_windowHandle);
        else XFixesHideCursor(m_display, m_windowHandle);
        XFlush(m_display);
    }
}

#endif
