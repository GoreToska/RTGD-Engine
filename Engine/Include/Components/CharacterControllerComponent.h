//
// Created by ivan on 8/28/26.
//

#pragma once

#include "Tools/Alias.h"

namespace RTGDEngine
{
    struct CharacterControllerComponent
    {
        enum class EMode
        {
            Physical,
            Virtual,
        };

        EMode Mode = EMode::Virtual;
        float MaxSlopeAngle = 50.0f;
        float Mass = 70.0f;
        float MaxStrength = 100.0f; // Only for virtual EMode - max push strength of dynamic bodies

        static void RegisterMeta(const World& world)
        {
            world.component<EMode>()
                    .constant("Physical", EMode::Physical)
                    .constant("Virtual", EMode::Virtual);

            world.component<CharacterControllerComponent>("CharacterControllerComponent")
                    .member<EMode>("Mode")
                    .member<float>("MaxSlopeAngle")
                    .member<float>("Mass")
                    .member<float>("MaxStrength");
        }
    };
}
