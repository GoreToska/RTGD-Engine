//
// Created by ivan on 6/22/26.
//

#if defined(_WIN32)
#include "Platform/Windows/EmbeddedWindowsWindow.h"

#include <windowsx.h>

#include "Tools/Logger.h"

namespace RTGDEngine {
    static constexpr UINT_PTR kSubclassId = 1;

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

        if (SetWindowSubclass(m_hwnd, &SubclassProc, kSubclassId, reinterpret_cast<DWORD_PTR>(this)))
            m_subclassed = true;

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
        if (m_subclassed && m_hwnd) {
            RemoveWindowSubclass(m_hwnd, &SubclassProc, kSubclassId);
            m_subclassed = false;
        }

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

    void EmbeddedWindowsWindow::SetRelativeMouseMode(bool relative) {
        if (relative) {
            std::lock_guard<std::mutex> lock(m_windowMutex);
            m_deltaX = 0.0f;
            m_deltaY = 0.0f;
        }
    }

    bool EmbeddedWindowsWindow::GetMouseDelta(float &dx, float &dy) {
        std::lock_guard<std::mutex> lock(m_windowMutex);
        dx = m_deltaX;
        dy = m_deltaY;
        m_deltaX = 0.0f;
        m_deltaY = 0.0f;
        return true;
    }

    void EmbeddedWindowsWindow::InjectMouseMove(float dx, float dy) {
        std::lock_guard<std::mutex> lock(m_windowMutex);
        m_deltaX += dx;
        m_deltaY += dy;
    }

    void EmbeddedWindowsWindow::WarpCursorToCenter() {
        RECT rc{};
        GetClientRect(m_hwnd, &rc);
        POINT center{(rc.right - rc.left) / 2, (rc.bottom - rc.top) / 2};
        ClientToScreen(m_hwnd, &center);
        SetCursorPos(center.x, center.y);
    }

    LRESULT EmbeddedWindowsWindow::SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                                                UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
        switch (uMsg) {
            case WM_MOUSEMOVE:
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
            case WM_LBUTTONDBLCLK: {
                HWND target = GetAncestor(hWnd, GA_ROOT);
                POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
                MapWindowPoints(hWnd, target, &pt, 1);
                PostMessage(target, uMsg, wParam, MAKELPARAM(pt.x, pt.y));
                break;
            }
            case WM_MOUSEACTIVATE:
                return MA_NOACTIVATE;
        }
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }
}

#endif
