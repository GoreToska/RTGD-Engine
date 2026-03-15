//
// Created by gorev on 13.03.2026.
//

#pragma once
#include <vector>

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

        static void BindStandardConstantBuffers(Diligent::IShaderResourceBinding& srb);

        static std::vector<Diligent::ShaderResourceVariableDesc> GetStandardVariableDescs()
        {
            return {
                {
                    Diligent::SHADER_TYPE_VERTEX, "CameraConstants",
                    Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE
                },
                {
                    Diligent::SHADER_TYPE_VERTEX, "ObjectConstants",
                    Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE
                },
            };
        }
    };
} // RTGDEngine
