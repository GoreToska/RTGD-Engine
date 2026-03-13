//
// Created by gorev on 13.03.2026.
//

#pragma once
#include "RenderDevice.h"
#include "RenderHandle.h"
#include "Engine/EngineExport.h"


namespace RTGDEngine
{
    class ENGINE_API PipelineFactory
    {
    public:
        static MaterialHandle CreateTrianglePipeline(
            Diligent::IRenderDevice& device,
            Diligent::ISwapChain& swapChain,
            const std::string& shadersPath);
    };
} // RTGDEngine
