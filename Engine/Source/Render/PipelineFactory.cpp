//
// Created by gorev on 13.03.2026.
//

#include "Render/PipelineFactory.h"

#include "AssetLoader/PathResolve.h"
#include "Render/RenderResourceManager.h"
#include "Render/RenderSystem.h"
#include "Render/Vertex.h"
#include "Tools/Logger.h"

namespace RTGDEngine {
    MaterialHandle PipelineFactory::CreateTrianglePipeline(Diligent::IRenderDevice &device,
                                                           Diligent::ISwapChain &swapChain,
                                                           const std::string &absolutePath) {
        using namespace Diligent;

        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderFactory;
        RTGDRenderSystem::Instance().GetFactory().CreateDefaultShaderSourceStreamFactory(
            absolutePath.c_str(), &pShaderFactory);

        ShaderCreateInfo shaderCI;
        shaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        shaderCI.pShaderSourceStreamFactory = pShaderFactory;
        shaderCI.Desc.UseCombinedTextureSamplers = true;

        RefCntAutoPtr<IShader> pVS;
        shaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
        shaderCI.Desc.Name = "Triangle VS";
        shaderCI.FilePath = "TriangleVS.hlsl";
        device.CreateShader(shaderCI, &pVS);

        if (!pVS) {
            LogError("Failed to create Triangle VS");
            return INVALID_MATERIAL_HANDLE;
        }

        RefCntAutoPtr<IShader> pPS;
        shaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
        shaderCI.Desc.Name = "Triangle PS";
        shaderCI.FilePath = "TrianglePS.hlsl";
        device.CreateShader(shaderCI, &pPS);

        if (!pPS) {
            LogError("Failed to create Triangle PS");
            return INVALID_MATERIAL_HANDLE;
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

        if (!data.PSO) {
            LogError("Failed to create Triangle PSO");
            return INVALID_MATERIAL_HANDLE;
        }

        data.PSO->CreateShaderResourceBinding(&data.SRB, true);

        BindStandardConstantBuffers(*data.SRB);

        return RenderResourceManager::Instance().RegisterMaterial("triangle", std::move(data));
    }

    MaterialHandle PipelineFactory::CreateMeshPipeline(Diligent::IRenderDevice &device, Diligent::ISwapChain &swapChain,
                                                       const std::string &absolutePath) {
        using namespace Diligent;

        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderFactory;
        RTGDRenderSystem::Instance().GetFactory().CreateDefaultShaderSourceStreamFactory(
            absolutePath.c_str(), &pShaderFactory);

        ShaderCreateInfo shaderCI;
        shaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        shaderCI.pShaderSourceStreamFactory = pShaderFactory;
        shaderCI.Desc.UseCombinedTextureSamplers = false;

        RefCntAutoPtr<IShader> pVS;
        shaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
        shaderCI.Desc.Name = "Mesh VS";
        shaderCI.FilePath = "MeshVS.hlsl";
        device.CreateShader(shaderCI, &pVS);
        if (!pVS) {
            LogError("Failed to create Mesh VS");
            return INVALID_MATERIAL_HANDLE;
        }

        RefCntAutoPtr<IShader> pPS;
        shaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
        shaderCI.Desc.Name = "Mesh PS";
        shaderCI.FilePath = "MeshPS.hlsl";
        device.CreateShader(shaderCI, &pPS);
        if (!pPS) {
            LogError("Failed to create Mesh PS");
            return INVALID_MATERIAL_HANDLE;
        }

        GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = "Mesh PSO";
        psoCI.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;
        psoCI.pVS = pVS;
        psoCI.pPS = pPS;

        auto layout = VertexLayout::PNTUV();
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
        if (!data.PSO) {
            LogError("Failed to create Mesh PSO");
            return INVALID_MATERIAL_HANDLE;
        }

        data.PSO->CreateShaderResourceBinding(&data.SRB, true);
        BindStandardConstantBuffers(*data.SRB);

        return RenderResourceManager::Instance().RegisterMaterial("mesh_default", std::move(data));
    }

    MaterialHandle PipelineFactory::CreateShadowPipeline(Diligent::IRenderDevice &device,
                                                         const std::string &absolutePath) {
        using namespace Diligent;

        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderFactory;
        RTGDRenderSystem::Instance().GetFactory()
                .CreateDefaultShaderSourceStreamFactory(absolutePath.c_str(), &pShaderFactory);

        ShaderCreateInfo shaderCI;
        shaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        shaderCI.pShaderSourceStreamFactory = pShaderFactory;
        shaderCI.Desc.UseCombinedTextureSamplers = false;

#ifdef RTGD_EDITOR
        ShaderMacro macros[] = {{"RTGD_EDITOR", "1"}};
        shaderCI.Macros = {macros, 1};
#endif

        RefCntAutoPtr<IShader> pVS;
        shaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
        shaderCI.Desc.Name = "Shadow VS";
        shaderCI.FilePath = "ShadowVS.hlsl";
        device.CreateShader(shaderCI, &pVS);
        if (!pVS) {
            LogError("Failed to create Shadow VS");
            return INVALID_MATERIAL_HANDLE;
        }

        GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = "Shadow PSO";
        psoCI.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;
        psoCI.pVS = pVS;
        psoCI.pPS = nullptr;

        auto layout = VertexLayout::PNTUV();
        psoCI.GraphicsPipeline.InputLayout.LayoutElements = layout.data();
        psoCI.GraphicsPipeline.InputLayout.NumElements = static_cast<uint32_t>(layout.size());
        psoCI.GraphicsPipeline.NumRenderTargets = 0;
        psoCI.GraphicsPipeline.DSVFormat = TEX_FORMAT_D32_FLOAT;
        psoCI.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        psoCI.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_FRONT;

        // If there will be issues with flat geometry - use CULL_MODE_BACK, but there will be peter panning
        psoCI.GraphicsPipeline.DepthStencilDesc.DepthEnable = True;
        psoCI.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = True;

        ShaderResourceVariableDesc vars[] =
        {
            {SHADER_TYPE_VERTEX, "ObjectConstants", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_VERTEX, "ShadowConstants", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };

        psoCI.PSODesc.ResourceLayout.Variables = vars;
        psoCI.PSODesc.ResourceLayout.NumVariables = std::size(vars);

        MaterialData data;
        device.CreateGraphicsPipelineState(psoCI, &data.PSO);
        if (!data.PSO) {
            LogError("Failed to create Shadow PSO");
            return INVALID_MATERIAL_HANDLE;
        }

        data.PSO->CreateShaderResourceBinding(&data.SRB, true);

        BindStandardConstantBuffers(*data.SRB);

        auto *shadowVar = data.SRB->GetVariableByName(SHADER_TYPE_VERTEX, "ShadowConstants");
        if (shadowVar)
            shadowVar->Set(&RTGDRenderSystem::Instance().GetFrameConstants().Shadow());

        return RenderResourceManager::Instance().RegisterMaterial("shadow", std::move(data));
    }

    MaterialHandle PipelineFactory::CreateGBufferPipeline(Diligent::IRenderDevice &device,
                                                          const std::string &absolutePath) {
        using namespace Diligent;

        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderFactory;
        RTGDRenderSystem::Instance().GetFactory()
                .CreateDefaultShaderSourceStreamFactory(absolutePath.c_str(), &pShaderFactory);

        ShaderCreateInfo shaderCI;
        shaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        shaderCI.pShaderSourceStreamFactory = pShaderFactory;
        shaderCI.Desc.UseCombinedTextureSamplers = false;

#ifdef RTGD_EDITOR
        ShaderMacro macros[] = {{"RTGD_EDITOR", "1"}};
        shaderCI.Macros = {macros, 1};
#endif

        RefCntAutoPtr<IShader> pVS;
        shaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
        shaderCI.Desc.Name = "GBuffer VS";
        shaderCI.FilePath = "GBufferVS.hlsl";
        device.CreateShader(shaderCI, &pVS);
        if (!pVS) {
            LogError("Failed to create GBuffer VS");
            return INVALID_MATERIAL_HANDLE;
        }

        RefCntAutoPtr<IShader> pPS;
        shaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
        shaderCI.Desc.Name = "GBuffer PS";
        shaderCI.FilePath = "GBufferPS.hlsl";

        device.CreateShader(shaderCI, &pPS);
        if (!pPS) {
            LogError("Failed to create GBuffer PS");
            return INVALID_MATERIAL_HANDLE;
        }

        GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = "GBuffer PSO";
        psoCI.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;
        psoCI.pVS = pVS;
        psoCI.pPS = pPS;

        auto layout = VertexLayout::PNTUV();
        psoCI.GraphicsPipeline.InputLayout.LayoutElements = layout.data();
        psoCI.GraphicsPipeline.InputLayout.NumElements = static_cast<uint32_t>(layout.size());

        psoCI.GraphicsPipeline.RTVFormats[0] = TEX_FORMAT_RGBA8_UNORM_SRGB; // Albedo
        psoCI.GraphicsPipeline.RTVFormats[1] = TEX_FORMAT_RGBA16_FLOAT; // Normal
        psoCI.GraphicsPipeline.RTVFormats[2] = TEX_FORMAT_RGBA32_FLOAT; // Position
        psoCI.GraphicsPipeline.RTVFormats[3] = TEX_FORMAT_RGBA8_UNORM; // PBR
#ifdef RTGD_EDITOR
        psoCI.GraphicsPipeline.RTVFormats[4] = TEX_FORMAT_R32_UINT;
        psoCI.GraphicsPipeline.NumRenderTargets = 5;
#else
        psoCI.GraphicsPipeline.NumRenderTargets = 4;
#endif

        psoCI.GraphicsPipeline.DSVFormat = TEX_FORMAT_D32_FLOAT;

        psoCI.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        psoCI.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_BACK;
        psoCI.GraphicsPipeline.DepthStencilDesc.DepthEnable = True;

        ShaderResourceVariableDesc vars[] =
        {
            {SHADER_TYPE_VERTEX, "CameraConstants", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_VERTEX, "ObjectConstants", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "g_Diffuse", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
            {SHADER_TYPE_PIXEL, "g_Normal", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
            {SHADER_TYPE_PIXEL, "g_MetallicRoughness", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
            {SHADER_TYPE_PIXEL, "g_AO", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
            {SHADER_TYPE_PIXEL, "g_Sampler", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
        };
        psoCI.PSODesc.ResourceLayout.Variables = vars;
        psoCI.PSODesc.ResourceLayout.NumVariables = std::size(vars);

        MaterialData data;
        device.CreateGraphicsPipelineState(psoCI, &data.PSO);
        if (!data.PSO) {
            LogError("Failed to create GBuffer PSO");
            return INVALID_MATERIAL_HANDLE;
        }

        data.PSO->CreateShaderResourceBinding(&data.SRB, true);
        BindStandardConstantBuffers(*data.SRB);

        return RenderResourceManager::Instance().RegisterMaterial(
            "gbuffer", std::move(data));
    }

    MaterialHandle PipelineFactory::CreateLightingPipeline(Diligent::IRenderDevice &device,
                                                           Diligent::ISwapChain &swapChain,
                                                           const std::string &absolutePath) {
        using namespace Diligent;

        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderFactory;
        RTGDRenderSystem::Instance().GetFactory()
                .CreateDefaultShaderSourceStreamFactory(absolutePath.c_str(), &pShaderFactory);

        ShaderCreateInfo shaderCI;
        shaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        shaderCI.pShaderSourceStreamFactory = pShaderFactory;
        shaderCI.Desc.UseCombinedTextureSamplers = false;

        RefCntAutoPtr<IShader> pVS;
        shaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
        shaderCI.Desc.Name = "Lighting VS";
        shaderCI.FilePath = "LightingVS.hlsl";
        device.CreateShader(shaderCI, &pVS);
        if (!pVS) {
            LogError("Failed to create Lighting VS");
            return INVALID_MATERIAL_HANDLE;
        }

        RefCntAutoPtr<IShader> pPS;
        shaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
        shaderCI.Desc.Name = "Lighting PS";
        shaderCI.FilePath = "LightingPS.hlsl";
        device.CreateShader(shaderCI, &pPS);
        if (!pPS) {
            LogError("Failed to create Lighting PS");
            return INVALID_MATERIAL_HANDLE;
        }

        GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = "Lighting PSO";
        psoCI.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;
        psoCI.pVS = pVS;
        psoCI.pPS = pPS;

        psoCI.GraphicsPipeline.InputLayout.NumElements = 0;

        psoCI.GraphicsPipeline.NumRenderTargets = 1;
        psoCI.GraphicsPipeline.RTVFormats[0] = swapChain.GetDesc().ColorBufferFormat;

        psoCI.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        psoCI.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_NONE;
        psoCI.GraphicsPipeline.DepthStencilDesc.DepthEnable = False;
        psoCI.GraphicsPipeline.DSVFormat = TEX_FORMAT_UNKNOWN;

        ShaderResourceVariableDesc vars[] =
        {
            {SHADER_TYPE_PIXEL, "LightConstants", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "CameraConstants", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "g_Diffuse", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
            {SHADER_TYPE_PIXEL, "g_Normal", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
            {SHADER_TYPE_PIXEL, "g_Position", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
            {SHADER_TYPE_PIXEL, "g_PBR", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
            {SHADER_TYPE_PIXEL, "g_Sampler", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
        };
        psoCI.PSODesc.ResourceLayout.Variables = vars;
        psoCI.PSODesc.ResourceLayout.NumVariables = std::size(vars);

        MaterialData data;
        device.CreateGraphicsPipelineState(psoCI, &data.PSO);
        if (!data.PSO) {
            LogError("Failed to create Lighting PSO");
            return INVALID_MATERIAL_HANDLE;
        }

        data.PSO->CreateShaderResourceBinding(&data.SRB, true);

        auto bindVar = [&](SHADER_TYPE type, const char *name, IDeviceObject *obj) {
            auto *var = data.SRB->GetVariableByName(type, name);
            if (var && obj)
                var->Set(obj);
        };

        bindVar(SHADER_TYPE_PIXEL, "LightConstants", &RTGDRenderSystem::Instance().GetFrameConstants().Light());
        bindVar(SHADER_TYPE_PIXEL, "CameraConstants", &RTGDRenderSystem::Instance().GetFrameConstants().Camera());

        return RenderResourceManager::Instance().RegisterMaterial(
            "lighting", std::move(data));
    }

    MaterialHandle PipelineFactory::CreateDebugViewPipeline(Diligent::IRenderDevice &device,
                                                            Diligent::ISwapChain &swapChain,
                                                            const std::string &absolutePath) {
        using namespace Diligent;

        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderFactory;
        RTGDRenderSystem::Instance().GetFactory()
                .CreateDefaultShaderSourceStreamFactory(absolutePath.c_str(), &pShaderFactory);

        ShaderCreateInfo shaderCI;
        shaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        shaderCI.pShaderSourceStreamFactory = pShaderFactory;
        shaderCI.Desc.UseCombinedTextureSamplers = false;

        RefCntAutoPtr<IShader> pVS;
        shaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
        shaderCI.Desc.Name = "Debug view VS";
        shaderCI.FilePath = "LightingVS.hlsl";
        device.CreateShader(shaderCI, &pVS);
        if (!pVS) {
            LogError("Failed to create Lighting VS");
            return INVALID_MATERIAL_HANDLE;
        }

        RefCntAutoPtr<IShader> pPS;
        shaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
        shaderCI.Desc.Name = "Debug view PS";
        shaderCI.FilePath = "DebugViewPS.hlsl";
        device.CreateShader(shaderCI, &pPS);
        if (!pPS) {
            LogError("Failed to create debug view PS");
            return INVALID_MATERIAL_HANDLE;
        }

        GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = "Debug PSO";
        psoCI.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;
        psoCI.pVS = pVS;
        psoCI.pPS = pPS;

        psoCI.GraphicsPipeline.InputLayout.NumElements = 0;

        psoCI.GraphicsPipeline.NumRenderTargets = 1;
        psoCI.GraphicsPipeline.RTVFormats[0] = swapChain.GetDesc().ColorBufferFormat;

        psoCI.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        psoCI.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_NONE;
        psoCI.GraphicsPipeline.DepthStencilDesc.DepthEnable = False;
        psoCI.GraphicsPipeline.DSVFormat = TEX_FORMAT_UNKNOWN;

        ShaderResourceVariableDesc vars[] =
        {
            {SHADER_TYPE_PIXEL, "g_Source", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
            {SHADER_TYPE_PIXEL, "g_Sampler", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
        };
        psoCI.PSODesc.ResourceLayout.Variables = vars;
        psoCI.PSODesc.ResourceLayout.NumVariables = std::size(vars);

        MaterialData data;
        device.CreateGraphicsPipelineState(psoCI, &data.PSO);
        if (!data.PSO) {
            LogError("Failed to create Lighting PSO");
            return INVALID_MATERIAL_HANDLE;
        }

        data.PSO->CreateShaderResourceBinding(&data.SRB, true);

        return RenderResourceManager::Instance().RegisterMaterial(
            "debug_view", std::move(data));
    }

    MaterialHandle PipelineFactory::CreateCompositePipeline(Diligent::IRenderDevice &device,
                                                            Diligent::ISwapChain &swapChain,
                                                            const std::string &absolutePath) {
        using namespace Diligent;

        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderFactory;
        RTGDRenderSystem::Instance().GetFactory()
                .CreateDefaultShaderSourceStreamFactory(absolutePath.c_str(), &pShaderFactory);

        ShaderCreateInfo shaderCI;
        shaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        shaderCI.pShaderSourceStreamFactory = pShaderFactory;
        shaderCI.Desc.UseCombinedTextureSamplers = false;

        RefCntAutoPtr<IShader> pVS;
        shaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
        shaderCI.Desc.Name = "Lighting VS";
        shaderCI.FilePath = "LightingVS.hlsl";
        device.CreateShader(shaderCI, &pVS);
        if (!pVS) {
            LogError("Failed to create Lighting VS");
            return INVALID_MATERIAL_HANDLE;
        }

        RefCntAutoPtr<IShader> pPS;
        shaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
        shaderCI.Desc.Name = "Composite PS";
        shaderCI.FilePath = "CompositePS.hlsl";
        device.CreateShader(shaderCI, &pPS);
        if (!pPS) {
            LogError("Failed to create Composite PS");
            return INVALID_MATERIAL_HANDLE;
        }

        GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = "Composite PSO";
        psoCI.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;
        psoCI.pVS = pVS;
        psoCI.pPS = pPS;

        psoCI.GraphicsPipeline.InputLayout.NumElements = 0;

        psoCI.GraphicsPipeline.NumRenderTargets = 1;
        psoCI.GraphicsPipeline.RTVFormats[0] = swapChain.GetDesc().ColorBufferFormat;

        psoCI.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        psoCI.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_NONE;
        psoCI.GraphicsPipeline.DepthStencilDesc.DepthEnable = False;
        psoCI.GraphicsPipeline.DSVFormat = TEX_FORMAT_UNKNOWN;

        ShaderResourceVariableDesc vars[] =
        {
            {SHADER_TYPE_PIXEL, "g_SceneColor", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
        };

        psoCI.PSODesc.ResourceLayout.Variables = vars;
        psoCI.PSODesc.ResourceLayout.NumVariables = std::size(vars);

        SamplerDesc samDesc;
        samDesc.MinFilter = FILTER_TYPE_POINT;
        samDesc.MagFilter = FILTER_TYPE_POINT;
        samDesc.MipFilter = FILTER_TYPE_POINT;
        samDesc.AddressU = TEXTURE_ADDRESS_CLAMP;
        samDesc.AddressV = TEXTURE_ADDRESS_CLAMP;
        samDesc.AddressW = TEXTURE_ADDRESS_CLAMP;
        ImmutableSamplerDesc immSamplers[] = {
            {SHADER_TYPE_PIXEL, "g_Sampler", samDesc},
        };

        psoCI.PSODesc.ResourceLayout.ImmutableSamplers = immSamplers;
        psoCI.PSODesc.ResourceLayout.NumImmutableSamplers = std::size(immSamplers);

        MaterialData data;
        device.CreateGraphicsPipelineState(psoCI, &data.PSO);
        if (!data.PSO) {
            LogError("Failed to create Lighting PSO");
            return INVALID_MATERIAL_HANDLE;
        }

        data.PSO->CreateShaderResourceBinding(&data.SRB, true);

        return RenderResourceManager::Instance().RegisterMaterial(
            "lighting", std::move(data));
    }

    void PipelineFactory::BindStandardConstantBuffers(Diligent::IShaderResourceBinding &srb) {
        using namespace Diligent;

        auto &rs = RTGDRenderSystem::Instance();

        struct Binding {
            const char *name;
            IBuffer *buffer;
        };

        const Binding bindings[] =
        {
            {"CameraConstants", &rs.GetFrameConstants().Camera()},
            {"ObjectConstants", &rs.GetFrameConstants().Object()},
            {"LightConstants", &rs.GetFrameConstants().Light()},
        };

        for (const auto &[name, buffer]: bindings) {
            if (!buffer)
                continue;

            for (auto shaderType: {SHADER_TYPE_VERTEX, SHADER_TYPE_PIXEL}) {
                auto *var = srb.GetVariableByName(shaderType, name);
                if (var) {
                    var->Set(buffer);
                    LogInfo("Bound '{}' to shader stage {}", name,
                            shaderType == SHADER_TYPE_VERTEX ? "VS" : "PS");
                }
            }
        }
    }
} // RTGDEngine
