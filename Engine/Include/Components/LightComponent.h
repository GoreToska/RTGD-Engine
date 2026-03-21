//
// Created by gorev on 17.03.2026.
//

#pragma once
#include "Tools/Alias.h"

namespace RTGDEngine
{
    struct AmbientLightComponent
    {
        Float3 Color = {1.0f, 1.0f, 1.0f};
        float Intensity = 0.1f;
    };

    struct DirectionalLightComponent
    {
        Float3 Direction = {0.0f, -1.0f, 0.0f};
        Float3 Color = {1.0f, 1.0f, 1.0f};
        float Intensity = 1.0f;
    };

    struct PointLightComponent
    {
        Float3 Color = {1.0f, 1.0f, 1.0f};
        float Intensity = 1.0f;
        float Radius = 10.0f;
    };

    struct SpotLightComponent
    {
        Float3 Direction = {0.0f, -1.0f, 0.0f};
        Float3 Color = {1.0f, 1.0f, 1.0f};
        float Intensity = 1.0f;
        float InnerAngle = 15.0f; // degrees
        float OuterAngle = 30.0f; // degrees
        float Radius = 20.0f;
    };
}
