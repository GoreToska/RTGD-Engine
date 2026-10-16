//
// Created by ivan on 10/15/26.
//

#pragma once
#include <cstdint>

#include "gainput/gainput.h"

namespace RTGDEngine
{
    enum class EInputEventType : uint8_t
    {
        MouseMove,
        MouseButton,
        MouseWheel,
        MouseLeave,
        Key,
        Text,
        FocusLost,
    };

    struct InputEvent
    {
        EInputEventType Type;
        bool Down = false;
        gainput::Key Key = gainput::KeyCount_;
        gainput::MouseButton Button = gainput::MouseButtonCount_;
        int X = 0;
        int Y = 0;
        float Wheel = 0.0f;
        bool Shift = false;
        bool Ctrl = false;
        bool Alt = false;
        char Text[8] = {};
    };
} // RTGDEngine
