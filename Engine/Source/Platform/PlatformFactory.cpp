//
// Created by ivan on 6/10/26.
//

#include "Platform/PlatformFactory.h"

#ifdef _WIN32
#include "Platform/Windows/WindowsPlatformWindow.h"
#include "Platform/Windows/EmbeddedWindowsWindow.h"
#elif defined(__linux__)
#include "Platform/Linux/LinuxPlatformWindow.h"
#include "Platform/Linux/EmbeddedLinuxWindow.h"
#endif

namespace RTGDEngine {
    std::unique_ptr<IPlatformWindow> CreatePlatformWindow() {
#ifdef _WIN32
        return std::make_unique<WindowsPlatformWindow>();
#elif defined(__linux__)
        return std::make_unique<LinuxPlatformWindow>();
#else
        static_assert(false, "Unsupported platform");
#endif
    }

    std::unique_ptr<IPlatformWindow> CreateEmbeddedPlatformWindow(const NativeWindowHandle &windowHandle) {
#ifdef _WIN32
        auto window = std::make_unique<EmbeddedWindowsWindow>(static_cast<HWND>(windowHandle.hwnd));
        window->Create({"Embedded window", windowHandle.width, windowHandle.height});
        return window;
#elif defined(__linux__)
        auto window = std::make_unique<EmbeddedLinuxWindow>(windowHandle.window);
        window->Create({"Embedded window", windowHandle.width, windowHandle.height});
        return window;
#endif
    }
}
