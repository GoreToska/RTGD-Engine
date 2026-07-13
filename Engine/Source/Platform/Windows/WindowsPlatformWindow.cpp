//
// Created by ivan on 6/10/26.
//

#include "Platform/Windows/WindowsPlatformWindow.h"
#if defined(_WIN32)

namespace RTGDEngine {
    constexpr const char *kWindowClassName = "RTGDEngineWindow";


    bool WindowsPlatformWindow::Create(const WindowDesc &desc) {
        m_hinstance = GetModuleHandle(nullptr);

        WNDCLASSEXA wc{};
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = &WindowsPlatformWindow::WndProc;
        wc.hInstance = m_hinstance;
        wc.hCursor = LoadCursorA(nullptr, IDC_ARROW);
        wc.hbrBackground = nullptr; // we render the whole surface ourselves
        wc.lpszClassName = kWindowClassName;

        // Registering the same class twice is ok
        if (!RegisterClassExA(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
            return false;
        }

        m_width = desc.width;
        m_height = desc.height;

        constexpr DWORD style = WS_OVERLAPPEDWINDOW;
        RECT rc{0, 0, desc.width, desc.height};
        AdjustWindowRect(&rc, style, FALSE);

        m_hwnd = CreateWindowExA(
            0,
            kWindowClassName,
            desc.title,
            style,
            CW_USEDEFAULT, CW_USEDEFAULT,
            rc.right - rc.left, rc.bottom - rc.top,
            nullptr, nullptr, m_hinstance,
            this
        );

        if (!m_hwnd) {
            return false;
        }

        ShowWindow(m_hwnd, SW_SHOW);
        UpdateWindow(m_hwnd);
        return true;
    }

    bool WindowsPlatformWindow::PollEvents() {
        MSG msg = {};
        while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                m_running = false;
                break;
            }

            if (OnNativeEvent) {
                NativeWindowEvent event{};
                event.Hwnd = m_hwnd;
                event.Msg = msg;
                OnNativeEvent(event);
            }

            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        return m_running;
    }

    NativeWindowHandle WindowsPlatformWindow::GetHandle() const {
        NativeWindowHandle handle;
        handle.hwnd = m_hwnd;
        handle.hinstance = m_hinstance;
        handle.width = m_width;
        handle.height = m_height;
        return handle;
    }

    EInputSource WindowsPlatformWindow::GetInputSource() const {
        return EInputSource::NativeEvents;
    }

    void WindowsPlatformWindow::Destroy() {
        if (m_hwnd) {
            DestroyWindow(m_hwnd);
            m_hwnd = nullptr;
        }
    }

    void WindowsPlatformWindow::SetCursorVisible(const bool visible) {
    }

    LRESULT CALLBACK WindowsPlatformWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (msg == WM_NCCREATE) {
            auto *cs = reinterpret_cast<CREATESTRUCTA *>(lParam);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
            return DefWindowProcA(hwnd, msg, wParam, lParam);
        }

        auto *self = reinterpret_cast<WindowsPlatformWindow *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (!self) {
            return DefWindowProcA(hwnd, msg, wParam, lParam);
        }

        switch (msg) {
            case WM_SIZE: {
                const int width = LOWORD(lParam);
                const int height = HIWORD(lParam);
                if (width > 0 && height > 0) {
                    self->m_width = width;
                    self->m_height = height;
                    if (self->OnResize)
                        self->OnResize(width, height);
                }
                break;
            }
            case WM_CLOSE:
                if (self->OnClose)
                    self->OnClose();
                self->m_running = false;
                DestroyWindow(hwnd);
                return 0;
            case WM_DESTROY:
                self->m_running = false;
                PostQuitMessage(0);
                return 0;
            default:
                break;
        }

        return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
} // RTGDEngine

#endif
