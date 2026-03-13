//
// Created by gorev on 12.03.2026.
//

#pragma once

#include "Tools/Alias.h"

namespace RTGDEngine
{
    struct VertexPC
    {
        Float3 Position;
        Float3 Color;
    };

    struct VertexPNUV
    {
        Float3 Position;
        Float3 Normal;
        Float2 UV;
    };

    namespace VertexLayout
    {
        constexpr std::vector<Diligent::LayoutElement> PC()
        {
            return
            {
                {0, 0, 3, Diligent::VT_FLOAT32, false}, // position
                {1, 0, 3, Diligent::VT_FLOAT32, false}, // color
            };
        }

        constexpr std::vector<Diligent::LayoutElement> PNUV()
        {
            return
            {
                {0, 0, 3, Diligent::VT_FLOAT32, false}, // position
                {1, 0, 3, Diligent::VT_FLOAT32, false}, // normal
                {2, 0, 2, Diligent::VT_FLOAT32, false}, // uv
            };
        }
    }
}
