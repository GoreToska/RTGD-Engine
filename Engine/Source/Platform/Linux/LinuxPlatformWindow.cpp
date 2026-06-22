//
// Created by ivan on 6/10/26.
//

#include "Platform/Linux/LinuxPlatformWindow.h"
#if defined(__linux__)
#include <X11/extensions/Xfixes.h>

namespace RTGDEngine {
    bool LinuxPlatformWindow::Create(const WindowDesc &desc) {
        m_display = XOpenDisplay(nullptr);
        if (!m_display) return false;

        int screen = DefaultScreen(m_display);

        m_width = desc.width;
        m_height = desc.height;

        m_window = XCreateSimpleWindow(
            m_display, RootWindow(m_display, screen),
            0, 0, desc.width, desc.height, 1,
            BlackPixel(m_display, screen),
            WhitePixel(m_display, screen)
        );

        XStoreName(m_display, m_window, desc.title);
        m_deleteAtom = XInternAtom(m_display, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(m_display, m_window, &m_deleteAtom, 1);
        XSelectInput(m_display, m_window,
                     ExposureMask | KeyPressMask | StructureNotifyMask | KeyReleaseMask | ButtonPressMask |
                     ButtonReleaseMask | PointerMotionMask);
        XMapWindow(m_display, m_window);
        return true;
    }

    bool LinuxPlatformWindow::PollEvents() {
        while (XPending(m_display)) {
            XEvent e;
            XNextEvent(m_display, &e);

            if (OnNativeEvent) {
                NativeWindowEvent event{};
                event.XEvent = &e;
                OnNativeEvent(event);
            }

            switch (e.type) {
                case ClientMessage:
                    if ((Atom) e.xclient.data.l[0] == m_deleteAtom) {
                        if (OnClose) OnClose();
                        m_running = false;
                    }
                    break;
                case ConfigureNotify:
                    if (OnResize)
                        OnResize(e.xconfigure.width, e.xconfigure.height);
                    break;
            }
        }

        return m_running;
    }

    NativeWindowHandle LinuxPlatformWindow::GetHandle() const {
        NativeWindowHandle handle;
        handle.display = m_display;
        handle.window = m_window;
        handle.width = m_width;
        handle.height = m_height;

        return handle;
    }

    EInputSource LinuxPlatformWindow::GetInputSource() const {
        return EInputSource::NativeEvents;
    }

    void LinuxPlatformWindow::Destroy() {
        XFreeCursor(m_display, m_window);

        if (m_window && m_display) {
            XDestroyWindow(m_display, m_window);
            m_window = 0;
        }
        if (m_display) {
            XCloseDisplay(m_display);
            m_display = nullptr;
        }
    }

    void LinuxPlatformWindow::SetCursorVisible(const bool visible) {
        if (visible) {
            XFixesShowCursor(m_display, m_window);
        } else {
            XFixesHideCursor(m_display, m_window);
        }
    }

    void LinuxPlatformWindow::SetMouseCapture(bool capture) {
        if (capture) {
            constexpr auto mask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
            XGrabPointer(m_display, m_window, capture, mask, GrabModeAsync, GrabModeAsync, m_window, None, CurrentTime);
        } else {
            XUngrabPointer(m_display, CurrentTime);
        }

        XFlush(m_display);
    }

    void LinuxPlatformWindow::CenterCursor() {
        const int centerX = m_width / 2;
        const int centerY = m_height / 2;
        XWarpPointer(m_display, None, m_window, 0, 0, 0, 0, centerX, centerY);
    }
} // RTGDEngine
#endif
