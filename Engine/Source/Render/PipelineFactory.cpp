//
// Created by gorev on 13.03.2026.
//

#include "Render/PipelineFactory.h"

#include "Render/RenderResourceManager.h"
#include "Render/RenderSystem.h"
#include "Render/Vertex.h"
#include "Tools/Logger.h"

namespace RTGDEngine
{
    MaterialHandle PipelineFactory::CreateTrianglePipeline(Diligent::IRenderDevice& device,
                                                           Diligent::ISwapChain& swapChain,
                                                           const std::string& shadersPath)
    {
        using namespace Diligent;

        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderFactory;
        RTGDRenderSystem::Instance().GetFactory().CreateDefaultShaderSourceStreamFactory(
            shadersPath.c_str(), &pShaderFactory);

        ShaderCreateInfo shaderCI;
        shaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        shaderCI.pShaderSourceStreamFactory = pShaderFactory;
        shaderCI.Desc.UseCombinedTextureSamplers = true;

        RefCntAutoPtr<IShader> pVS;
        shaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
        shaderCI.Desc.Name = "Triangle VS";
        shaderCI.FilePath = "TriangleVS.hlsl";
        device.CreateShader(shaderCI, &pVS);

        if (!pVS)
        {
            LogError("Failed to create Triangle VS");
            return INVALID_HANDLE;
        }

        RefCntAutoPtr<IShader> pPS;
        shaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
        shaderCI.Desc.Name = "Triangle PS";
        shaderCI.FilePath = "TrianglePS.hlsl";
        device.CreateShader(shaderCI, &pPS);

        if (!pPS)
        {
            LogError("Failed to create Triangle PS");
            return INVALID_HANDLE;
        }

        GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = "Triangle PSO";
        psoCI.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;
        psoCI.pVS = pVS;
        psoCI.pPS = pPS;

        auto layout = VertexLayout::PC();
        psoCI.GraphicsPipeline.InputLayout.LayoutElements = layout.data();
        psoCI.GraphicsPipeline.InputLayout.NumElements = static_cast<uint32_t>(layout.size());

        psoCI.GraphicsPipeline.NumRenderTargets = 1;
        psoCI.GraphicsPipeline.RTVFormats[0] = swapChain.GetDesc().ColorBufferFormat;
        psoCI.GraphicsPipeline.DSVFormat = swapChain.GetDesc().DepthBufferFormat;

        psoCI.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        psoCI.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_NONE;
        psoCI.GraphicsPipeline.DepthStencilDesc.DepthEnable = False;

        auto vars = GetStandardVariableDescs();
        psoCI.PSODesc.ResourceLayout.Variables = vars.data();
        psoCI.PSODesc.ResourceLayout.NumVariables = static_cast<uint32_t>(vars.size());

        MaterialData data;
        device.CreateGraphicsPipelineState(psoCI, &data.PSO);

        if (!data.PSO)
        {
            LogError("Failed to create Triangle PSO");
            return INVALID_HANDLE;
        }

        data.PSO->CreateShaderResourceBinding(&data.SRB, true);

        BindStandardConstantBuffers(*data.SRB);

        return RenderResourceManager::Instance().RegisterMaterial("triangle", std::move(data));
    }

    MaterialHandle PipelineFactory::CreateMeshPipeline(Diligent::IRenderDevice& device, Diligent::ISwapChain& swapChain,
                                                       const std::string& shadersPath)
    {
        using namespace Diligent;

        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderFactory;
        RTGDRenderSystem::Instance().GetFactory().CreateDefaultShaderSourceStreamFactory(
            shadersPath.c_str(), &pShaderFactory);

        ShaderCreateInfo shaderCI;
        shaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        shaderCI.pShaderSourceStreamFactory = pShaderFactory;
        shaderCI.Desc.UseCombinedTextureSamplers = true;

        RefCntAutoPtr<IShader> pVS;
        shaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
        shaderCI.Desc.Name = "Mesh VS";
        shaderCI.FilePath = "MeshVS.hlsl";
        device.CreateShader(shaderCI, &pVS);
        if (!pVS)
        {
            LogError("Failed to create Mesh VS");
            return INVALID_HANDLE;
        }

        RefCntAutoPtr<IShader> pPS;
        shaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
        shaderCI.Desc.Name = "Mesh PS";
        shaderCI.FilePath = "MeshPS.hlsl";
        device.CreateShader(shaderCI, &pPS);
        if (!pPS)
        {
            LogError("Failed to create Mesh PS");
            return INVALID_HANDLE;
        }

        GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = "Mesh PSO";
        psoCI.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;
        psoCI.pVS = pVS;
        psoCI.pPS = pPS;

        auto layout = VertexLayout::PNUV();
        psoCI.GraphicsPipeline.InputLayout.LayoutElements = layout.data();
        psoCI.GraphicsPipeline.InputLayout.NumElements = static_cast<uint32_t>(layout.size());

        psoCI.GraphicsPipeline.NumRenderTargets = 1;
        psoCI.GraphicsPipeline.RTVFormats[0] = swapChain.GetDesc().ColorBufferFormat;
        psoCI.GraphicsPipeline.DSVFormat = swapChain.GetDesc().DepthBufferFormat;
        psoCI.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        psoCI.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_BACK;
        psoCI.GraphicsPipeline.DepthStencilDesc.DepthEnable = True;

        auto vars = GetStandardVariableDescs();
        psoCI.PSODesc.ResourceLayout.Variables = vars.data();
        psoCI.PSODesc.ResourceLayout.NumVariables = static_cast<uint32_t>(vars.size());

        MaterialData data;
        device.CreateGraphicsPipelineState(psoCI, &data.PSO);
        if (!data.PSO)
        {
            LogError("Failed to create Mesh PSO");
            return INVALID_HANDLE;
        }

        data.PSO->CreateShaderResourceBinding(&data.SRB, true);
        BindStandardConstantBuffers(*data.SRB);

        return RenderResourceManager::Instance().RegisterMaterial("mesh_default", std::move(data));
    }

    void PipelineFactory::BindStandardConstantBuffers(Diligent::IShaderResourceBinding& srb)
    {
        using namespace Diligent;

        auto& rs = RTGDRenderSystem::Instance();

        struct Binding
        {
            const char* name;
            IBuffer* buffer;
        };

        const Binding bindings[] =
        {
            {"CameraConstants", &rs.GetCameraCB()},
            {"ObjectConstants", &rs.GetObjectCB()},
        };

        for (const auto& [name, buffer]: bindings)
        {
            if (!buffer)
                continue;

            for (auto shaderType: {SHADER_TYPE_VERTEX, SHADER_TYPE_PIXEL})
            {
                auto* var = srb.GetVariableByName(shaderType, name);
                if (var)
                {
                    var->Set(buffer);
                    LogInfo("Bound '{}' to shader stage {}", name,
                            shaderType == SHADER_TYPE_VERTEX ? "VS" : "PS");
                }
            }
        }
    }
} // RTGDEngine
