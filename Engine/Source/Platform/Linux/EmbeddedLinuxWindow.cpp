//
// Created by ivan on 6/16/26.
//

#if defined(__linux__)
#include "Platform/Linux/EmbeddedLinuxWindow.h"

#include <X11/extensions/Xfixes.h>
#include <X11/extensions/XInput2.h>

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

        int event, error;
        if (XQueryExtension(m_display, "XInputExtension", &m_xiOpcode, &event, &error)) {
            int major = 2, minor = 0;
            XIQueryVersion(m_display, &major, &minor);

            unsigned char mask[XIMaskLen(XI_LASTEVENT)] = {0};
            XISetMask(mask, XI_RawMotion);

            XIEventMask evmask;
            evmask.deviceid = XIAllMasterDevices;
            evmask.mask_len = sizeof(mask);
            evmask.mask = mask;
            XISelectEvents(m_display, DefaultRootWindow(m_display), &evmask, 1);
            XFlush(m_display);
        } else {
            LogError("XInput2 not available; mouse look disabled.");
        }
        return true;
    }

    bool EmbeddedLinuxWindow::PollEvents() {
        XEvent ev;
        while (XPending(m_display) > 0) {
            XNextEvent(m_display, &ev);

            if (ev.type != GenericEvent || ev.xcookie.extension != m_xiOpcode)
                continue;
            if (!XGetEventData(m_display, &ev.xcookie))
                continue;

            if (ev.xcookie.evtype == XI_RawMotion) {
                auto *re = static_cast<XIRawEvent *>(ev.xcookie.data);
                const double *val = re->raw_values;
                for (int i = 0; i < re->valuators.mask_len * 8; ++i) {
                    if (XIMaskIsSet(re->valuators.mask, i)) {
                        if (i == 0) m_deltaX += static_cast<float>(*val);
                        else if (i == 1) m_deltaY += static_cast<float>(*val);
                        ++val;
                    }
                }
            }

            XFreeEventData(m_display, &ev.xcookie);
        }
        return true;
    }

    bool EmbeddedLinuxWindow::GetMouseDelta(float &dx, float &dy) {
        dx = m_deltaX;
        dy = m_deltaY;
        m_deltaX = 0.0f;
        m_deltaY = 0.0f;
        return true;
    }

    void EmbeddedLinuxWindow::SetRelativeMouseMode(bool relative) {
        if (relative) {
            SetCursorVisible(false);
            XGrabPointer(m_display, m_windowHandle, True,
                         ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync,
                         m_windowHandle, None, CurrentTime);
            m_deltaX = 0.0f;
            m_deltaY = 0.0f;
        } else {
            XUngrabPointer(m_display, CurrentTime);
            SetCursorVisible(true);
        }

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
