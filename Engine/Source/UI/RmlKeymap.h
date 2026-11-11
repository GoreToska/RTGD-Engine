#pragma once
#include <RmlUi/Core/Input.h>

#include "Platform/InputEvent.h"

namespace RTGDEngine
{
    inline int GetKeyModifier(const InputEvent& event)
    {
        int modifier = 0;
        if (event.Alt)
            modifier |= Rml::Input::KM_ALT;
        if (event.Shift)
            modifier |= Rml::Input::KM_SHIFT;
        if (event.Ctrl)
            modifier |= Rml::Input::KM_CTRL;
        return modifier;
    }
} // RTGDEngine
