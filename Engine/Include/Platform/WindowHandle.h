//
// Created by ivan on 6/10/26.
//

#pragma once
namespace RTGDEngine {
    struct NativeWindowHandle {
        int width = 0;
        int height = 0;

#ifdef _WIN32
        void *hwnd = nullptr;
        void *hinstance = nullptr;
#else
        void *display = nullptr; // Display*
        unsigned long window = 0; // Window (XID)
#endif
    };
}
