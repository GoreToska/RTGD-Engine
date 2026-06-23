//
// Created by ivan on 6/15/26.
//

#pragma once

struct NativeWindowEvent {
#ifdef _WIN32
    void *Hwnd;
    unsigned Msg;
    unsigned long long WParam;
    unsigned long long LParam;
#elif __linux__
    void *XEvent;
#endif
};

enum class EInputSource {
    NativeEvents, // XEvent/MSG
    Injected // Injection by embedded host
};
