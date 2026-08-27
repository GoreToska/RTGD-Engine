//
// Created by ivan on 8/27/26.
//

#pragma once
#include <cstdint>

namespace RTGDEngine
{
    struct GroundCheckComponent
    {
        float Margin = 0.1f;
        uint32_t LayerMask = ~0u;
        bool IsGrounded = false;

        static void RegisterMeta(const World& world)
        {
            world.component<GroundCheckComponent>("GroundCheckComponent")
                    .member<float>("Margin")
                    .member<uint32_t>("LayerMask")
                    .member<bool>("IsGrounded");
        }
    };
}
