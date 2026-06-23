//
// Created by gorev on 12.03.2026.
//

#pragma once
#include <cstdint>

namespace RTGDEngine
{
    enum class RenderLayer : uint8_t
    {
        Opaque = 0,
        Transparent = 1,
        UI = 2,
        Debug = 3,
    };

    struct RenderComponent
    {
        bool IsVisible = true;
        bool CastShadows = true;
        RenderLayer Layer = RenderLayer::Opaque;

        static void RegisterMeta(const flecs::world& world)
        {
            flecs::component<RenderComponent>(world, "RenderComponent")
                    .member<bool>("IsVisible")
                    .member<bool>("CastShadows");
            // TODO: render layer
        }
    };
}
