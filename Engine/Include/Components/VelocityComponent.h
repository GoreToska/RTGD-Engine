//
// Created by gorev on 13.03.2026.
//

#pragma once
#include "Tools/Alias.h"

namespace RTGDEngine
{
    struct VelocityComponent
    {
        Float3 Linear = {0.0f, 0.0f, 0.0f};
        Float3 Angular = {0.0f, 0.0f, 0.0f}; // pitch, yaw, roll
    };
}
