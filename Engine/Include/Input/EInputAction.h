//
// Created by gorev on 13.03.2026.
//

#pragma once
#include <gainput/gainput.h>

namespace RTGDEngine
{
    enum class EInputAction : gainput::UserButtonId
    {
        MoveForward = 0,
        MoveBackward,
        MoveLeft,
        MoveRight,
        MoveUp,
        MoveDown,
        SpeedBoost,
        LookX,
        LookY,
        DeltaX,
        DeltaY,
        MouseRight,
        Escape,

        Count
    };

    inline gainput::UserButtonId ID(EInputAction action)
    {
        return static_cast<gainput::UserButtonId>(action);
    }
}
