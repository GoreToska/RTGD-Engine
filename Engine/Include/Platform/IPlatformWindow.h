//
// Created by ivan on 6/10/26.
//

#pragma once
#include <functional>

#include "WindowEvent.h"
#include "Platform/WindowHandle.h"

namespace RTGDEngine {
    struct WindowDesc {
        const char *title;
        int width;
        int height;
    };

    class IPlatformWindow {
    public:
        virtual ~IPlatformWindow() = default;

        virtual bool Create(const WindowDesc &desc) = 0;

        virtual void Destroy() = 0;

        virtual bool PollEvents() = 0; // false is exit

        [[nodiscard]] virtual NativeWindowHandle GetHandle() const = 0;

        [[nodiscard]] virtual EInputSource GetInputSource() const = 0;

        virtual void SetCursorVisible(bool visible) = 0;

        virtual void SetMouseCapture(bool capture) = 0;

        virtual void CenterCursor() = 0;

        virtual void WarpCursor(int x, int y) {
        };

        virtual bool GetMouseDelta(float &dx, float &dy) { return false; }

        [[nodiscard]] int GetWidth() const { return m_width; }
        [[nodiscard]] int GetHeight() const { return m_height; }

        void SetSize(int width, int height) {
            m_width = width;
            m_height = height;
        }

        std::function<void(int width, int height)> OnResize;
        std::function<void()> OnClose;
        std::function<void(const NativeWindowEvent &)> OnNativeEvent;

    protected:
        int m_width = 0;
        int m_height = 0;
    };
}
