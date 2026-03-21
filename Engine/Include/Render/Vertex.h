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

    struct VertexPNTUV
    {
        Float3 Position;
        Float3 Normal;
        Float4 Tangent; // W - handedness
        Float2 UV;
    };

    namespace VertexLayout
    {
        inline std::vector<Diligent::LayoutElement> PC()
        {
            return
            {
                {0, 0, 3, Diligent::VT_FLOAT32, false}, // position
                {1, 0, 3, Diligent::VT_FLOAT32, false}, // color
            };
        }

        inline std::vector<Diligent::LayoutElement> PNTUV()
        {
            constexpr Diligent::Uint32 stride = sizeof(VertexPNTUV);
            return {
                {0, 0, 3, Diligent::VT_FLOAT32, false, offsetof(VertexPNTUV, Position), stride},
                {1, 0, 3, Diligent::VT_FLOAT32, false, offsetof(VertexPNTUV, Normal), stride},
                {2, 0, 4, Diligent::VT_FLOAT32, false, offsetof(VertexPNTUV, Tangent), stride},
                {3, 0, 2, Diligent::VT_FLOAT32, false, offsetof(VertexPNTUV, UV), stride},
            };
        }
    }
}
