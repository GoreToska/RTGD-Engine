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

        static void RegisterMeta(const flecs::world& world)
        {
            flecs::component<AmbientLightComponent>(world, "AmbientLightComponent")
                    .member<Float3>("Color")
                    .member<float>("Intensity");
        }
    };

    struct DirectionalLightComponent
    {
        Float3 Direction = {0.0f, -1.0f, 0.0f};
        Float3 Color = {1.0f, 1.0f, 1.0f};
        float Intensity = 1.0f;

        static void RegisterMeta(const flecs::world& world)
        {
            flecs::component<DirectionalLightComponent>(world, "DirectionalLightComponent")
                    .member<Float3>("Direction")
                    .member<Float3>("Color")
                    .member<float>("Intensity");
        }
    };

    struct PointLightComponent
    {
        Float3 Color = {1.0f, 1.0f, 1.0f};
        float Intensity = 1.0f;
        float Radius = 10.0f;

        static void RegisterMeta(const flecs::world& world)
        {
            flecs::component<PointLightComponent>(world, "PointLightComponent")
                    .member<Float3>("Color")
                    .member<float>("Intensity")
                    .member<float>("Radius");
        }
    };

    struct SpotLightComponent
    {
        Float3 Direction = {0.0f, -1.0f, 0.0f};
        Float3 Color = {1.0f, 1.0f, 1.0f};
        float Intensity = 1.0f;
        float InnerAngle = 15.0f; // degrees
        float OuterAngle = 30.0f; // degrees
        float Radius = 20.0f;

        static void RegisterMeta(const flecs::world& world)
        {
            flecs::component<SpotLightComponent>(world, "SpotLightComponent")
                    .member<Float3>("Direction")
                    .member<Float3>("Color")
                    .member<float>("Intensity")
                    .member<float>("InnerAngle")
                    .member<float>("OuterAngle")
                    .member<float>("Radius");
        }
    };
}
