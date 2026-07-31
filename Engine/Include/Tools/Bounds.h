//
// Created by ivan on 7/31/26.
//

#pragma once
#include "Alias.h"

namespace RTGDEngine {
    struct BoundingSphere {
        Float3 Center = {0, 0, 0};
        float Radius = 0.0f;
    };

    struct AABB {
        Float3 Min = {0, 0, 0};
        Float3 Max = {0, 0, 0};
    };
}
