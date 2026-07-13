//
// Created by ivan on 6/10/26.
//

#include "Platform/Linux/LinuxPlatformWindow.h"
#if defined(__linux__)
#include <X11/extensions/Xfixes.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XInput2.h>

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

        int xiEvent;
        int xiError;

        if (XQueryExtension(m_display, "XInputExtension", &m_xiOpcode, &xiEvent, &xiError)) {
            int major = 2;
            int minor = 0;
            XIQueryVersion(m_display, &major, &minor);
            unsigned char mask[XIMaskLen(XI_LASTEVENT)] = {0};
            XISetMask(mask, XI_RawMotion);
            XIEventMask evMask{XIAllMasterDevices, sizeof(mask), mask};
            XISelectEvents(m_display, DefaultRootWindow(m_display), &evMask, 1);
            XFlush(m_display);
        }

        XMapWindow(m_display, m_window);

        Bool supported;
        XkbSetDetectableAutoRepeat(m_display, True, &supported);

        return true;
    }

    bool LinuxPlatformWindow::PollEvents() {
        while (XPending(m_display)) {
            XEvent e;
            XNextEvent(m_display, &e);

            if (e.type == GenericEvent && e.xcookie.extension == m_xiOpcode) {
                if (XGetEventData(m_display, &e.xcookie)) {
                    if (e.xcookie.evtype == XI_RawMotion) {
                        auto *re = static_cast<XIRawEvent *>(e.xcookie.data);
                        const double *val = re->raw_values;
                        for (int i = 0; i < re->valuators.mask_len * 8; ++i) {
                            if (XIMaskIsSet(re->valuators.mask, i)) {
                                if (i == 0) m_deltaX += static_cast<float>(*val);
                                else if (i == 1) m_deltaY += static_cast<float>(*val);
                                ++val;
                            }
                        }
                    }
                    XFreeEventData(m_display, &e.xcookie);
                }
                continue;
            }

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

    void LinuxPlatformWindow::SetRelativeMouseMode(bool relative) {
        if (relative) {
            SetCursorVisible(false);
            XGrabPointer(m_display, m_window, True,
                         ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                         GrabModeAsync, GrabModeAsync, m_window, None, CurrentTime);
            m_deltaX = 0.0f;
            m_deltaY = 0.0f;
        } else {
            XUngrabPointer(m_display, CurrentTime);
            SetCursorVisible(true);
        }
        XFlush(m_display);
    }

    bool LinuxPlatformWindow::GetMouseDelta(float &dx, float &dy) {
        dx = m_deltaX;
        dy = m_deltaY;
        m_deltaX = 0.0f;
        m_deltaY = 0.0f;
        return true;
    }
} // RTGDEngine
#endif
