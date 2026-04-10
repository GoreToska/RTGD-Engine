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

        static void RegisterMeta(const flecs::world& world)
        {
            flecs::component<VelocityComponent>(world, "VelocityComponent")
                    .member<Float3>("Linear")
                    .member<Float3>("Angular");
        }
    };
}
