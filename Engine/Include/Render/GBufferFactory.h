//
// Created by gorev on 17.03.2026.
//

#pragma once
#include "Engine/EngineExport.h"
#include "GBuffer.h"

namespace Diligent
{
    struct IRenderDevice;
}

namespace RTGDEngine
{
    class ENGINE_API GBufferFactory
    {
    public:
        static GBuffer Create(Diligent::IRenderDevice& device,
                              uint32_t width,
                              uint32_t height);

        static void Resize(GBuffer& gbuffer,
                           Diligent::IRenderDevice& device,
                           uint32_t width,
                           uint32_t height);
    };
} // RTGDEngine
