//
// Created by gorev on 13.03.2026.
//

#pragma once
#include <vector>

#include "RenderDevice.h"
#include "RenderHandle.h"
#include "Engine/EngineExport.h"


namespace RTGDEngine {
    struct GBuffer;

    class ENGINE_API PipelineFactory {
    public:
        static MaterialHandle CreateTrianglePipeline(
            Diligent::IRenderDevice &device,
            Diligent::ISwapChain &swapChain,
            const std::string &absolutePath);

        static MaterialHandle CreateMeshPipeline(
            Diligent::IRenderDevice &device,
            Diligent::ISwapChain &swapChain,
            const std::string &absolutePath);

        static MaterialHandle CreateGBufferPipeline(
            Diligent::IRenderDevice &device,
            const GBuffer &gbuffer,
            const std::string &absolutePath);

        static MaterialHandle CreateLightingPipeline(
            Diligent::IRenderDevice &device,
            Diligent::ISwapChain &swapChain,
            const std::string &absolutePath);

        static MaterialHandle CreateDebugViewPipeline(
            Diligent::IRenderDevice &device,
            Diligent::ISwapChain &swapChain,
            const std::string &absolutePath);

        static MaterialHandle CreateCompositePipeline(
            Diligent::IRenderDevice &device,
            Diligent::ISwapChain &swapChain,
            const std::string &absolutePath);

        static void BindStandardConstantBuffers(Diligent::IShaderResourceBinding &srb);

        static std::vector<Diligent::ShaderResourceVariableDesc> GetStandardVariableDescs() {
            return {
                {
                    Diligent::SHADER_TYPE_VERTEX, "CameraConstants",
                    Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE
                },
                {
                    Diligent::SHADER_TYPE_VERTEX, "ObjectConstants",
                    Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE
                },
                {
                    Diligent::SHADER_TYPE_PIXEL, "LightConstants",
                    Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE
                },
                {
                    Diligent::SHADER_TYPE_PIXEL, "g_Texture",
                    Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC
                },
                {
                    Diligent::SHADER_TYPE_PIXEL, "g_Sampler",
                    Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC
                },
            };
        }
    };
} // RTGDEngine
