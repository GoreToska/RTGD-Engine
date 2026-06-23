//
// Created by ivan on 6/22/26.
//

#if defined(_WIN32)
#include "Platform/Windows/EmbeddedWindowsWindow.h"

#include "Tools/Logger.h"

namespace RTGDEngine {
    EmbeddedWindowsWindow::EmbeddedWindowsWindow(HWND hwnd)
        : m_hwnd(hwnd) {
    }

    bool EmbeddedWindowsWindow::Create(const WindowDesc &desc) {
        if (!m_hwnd || !IsWindow(m_hwnd)) {
            LogError("EmbeddedWindowsWindow: invalid HWND passed in constructor.");
            return false;
        }

        // The HWND belongs to the editor
        m_hinstance = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE));

        RECT rc{};
        GetClientRect(m_hwnd, &rc);
        SetSize(rc.right - rc.left, rc.bottom - rc.top);

        LogInfo("EmbeddedWindowsWindow created ({}x{})", GetWidth(), GetHeight());
        return true;
    }

    bool EmbeddedWindowsWindow::PollEvents() {
        return true; // nothing to poll here.
    }

    NativeWindowHandle EmbeddedWindowsWindow::GetHandle() const {
        return {GetWidth(), GetHeight(), m_hwnd, m_hinstance};
    }

    EInputSource EmbeddedWindowsWindow::GetInputSource() const {
        return EInputSource::Injected;
    }

    void EmbeddedWindowsWindow::Destroy() {
        // We don't own the HWND so just drop our references.
        m_hwnd = nullptr;
        m_hinstance = nullptr;
    }

    void EmbeddedWindowsWindow::SetCursorVisible(bool visible) {
        if (visible) {
            while (ShowCursor(TRUE) < 0) {
            }
        } else {
            while (ShowCursor(FALSE) >= 0) {
            }
        }
    }

    void EmbeddedWindowsWindow::SetMouseCapture(bool capture) {
        if (capture) {
            SetCapture(m_hwnd);
            RECT rc;
            GetClientRect(m_hwnd, &rc);
            POINT tl{rc.left, rc.top}, br{rc.right, rc.bottom};
            ClientToScreen(m_hwnd, &tl);
            ClientToScreen(m_hwnd, &br);
            RECT clip{tl.x, tl.y, br.x, br.y};
            ClipCursor(&clip);
        } else {
            ReleaseCapture();
            ClipCursor(nullptr);
        }
    }

    void EmbeddedWindowsWindow::CenterCursor() {
    }
}

#endif
