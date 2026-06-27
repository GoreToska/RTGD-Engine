//
// Created by ivan on 6/15/26.
//

#pragma once

#ifdef _WIN32
#include <Windows.h>
#elif __linux__

#endif

struct NativeWindowEvent
{
#ifdef _WIN32
    void* Hwnd;
    MSG Msg;
#elif __linux__
    void *XEvent; // TODO: change to XEvent type
#endif
};

enum class EInputSource
{
    NativeEvents, // XEvent/MSG
    Injected // Injection by embedded host
};
