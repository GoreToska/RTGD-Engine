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
        RenderLayer RenderLayer = RenderLayer::Opaque;
    };
}
