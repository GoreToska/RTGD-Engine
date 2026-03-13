//
// Created by gorev on 12.03.2026.
//

#pragma once
#include "RenderHandle.h"

namespace Diligent
{
    struct IRenderDevice;
}

namespace RTGDEngine
{
    class SimpleMeshFactory
    {
    public:
        static MeshHandle CreateTriangle(Diligent::IRenderDevice& device);
    };
}
