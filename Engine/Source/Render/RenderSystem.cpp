#include "Render/RenderSystem.h"

#include <filesystem>

#include "Render/ShaderLoader.h"

#include <EngineFactoryD3D12.h>
#include <GraphicsTypes.h>
#include <RenderDevice.h>
#include <DeviceContext.h>
#include <SwapChain.h>
#include <PipelineState.h>
#include <Buffer.h>
#include <Shader.h>

#include "Tools/Logger.h"

namespace RTGDEngine
{
    RTGDRenderSystem& RTGDRenderSystem::Instance()
    {
        static RTGDRenderSystem render;
        return render;
    }

    bool RTGDRenderSystem::Initialize(void* hwnd, int width, int height)
    {
        using namespace Diligent;

        m_width = width;
        m_height = height;


        auto* GetEngineFactoryD3D12 = LoadGraphicsEngineD3D12();
        if (!GetEngineFactoryD3D12)
        {
            LogError("Failed to load GraphicsEngineD3D12.dll");
            return false;
        }

        auto* pFactoryD3D12 = GetEngineFactoryD3D12();

        EngineD3D12CreateInfo engineCI;
        engineCI.GraphicsAPIVersion = {12, 0};

#ifdef _DEBUG
        engineCI.SetValidationLevel(VALIDATION_LEVEL_2);
#endif

        SwapChainDesc scDesc;
        scDesc.Width = width;
        scDesc.Height = height;
        scDesc.ColorBufferFormat = TEX_FORMAT_RGBA8_UNORM_SRGB;
        scDesc.DepthBufferFormat = TEX_FORMAT_D32_FLOAT;

        Win32NativeWindow window{static_cast<HWND>(hwnd)};

        pFactoryD3D12->CreateDeviceAndContextsD3D12(
            engineCI, &m_pDevice, &m_pImmediateContext);

        pFactoryD3D12->CreateSwapChainD3D12(
            m_pDevice, m_pImmediateContext,
            scDesc, FullScreenModeDesc{}, window, &m_pSwapChain);

        if (!m_pDevice || !m_pImmediateContext || !m_pSwapChain)
        {
            LogError("Failed to create Diligent device/swapchain");
            return false;
        }

        m_pFactory = pFactoryD3D12;

        CreateTriangleVertexBuffer();
        CreateTrianglePipeline();


        m_initialized = true;
        return true;
    }

    struct TriangleVertex
    {
        float x, y;
        float r, g, b;
    };

    static const TriangleVertex kVerts[] = {
        {0.0f, 0.5f, 1.0f, 0.0f, 0.0f},
        {-0.5f, -0.5f, 0.0f, 1.0f, 0.0f},
        {0.5f, -0.5f, 0.0f, 0.0f, 1.0f},
    };

    void RTGDRenderSystem::CreateTrianglePipeline()
    {
        using namespace Diligent;

        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
        m_pFactory->CreateDefaultShaderSourceStreamFactory(
            "Shaders", &pShaderSourceFactory);

        ShaderCreateInfo shaderCI;
        shaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        shaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
        shaderCI.Desc.UseCombinedTextureSamplers = true;

        RefCntAutoPtr<IShader> pVS;
        shaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
        shaderCI.Desc.Name = "Triangle VS";
        shaderCI.FilePath = "TriangleVS.hlsl";
        m_pDevice->CreateShader(shaderCI, &pVS);

        RefCntAutoPtr<IShader> pPS;
        shaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
        shaderCI.Desc.Name = "Triangle PS";
        shaderCI.FilePath = "TrianglePS.hlsl";
        m_pDevice->CreateShader(shaderCI, &pPS);

        GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = "Triangle PSO";
        psoCI.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;
        psoCI.pVS = pVS;
        psoCI.pPS = pPS;

        LayoutElement layout[] = {
            {0, 0, 2, VT_FLOAT32, False}, // float2 Position
            {1, 0, 3, VT_FLOAT32, False}, // float3 Color
        };
        psoCI.GraphicsPipeline.InputLayout.LayoutElements = layout;
        psoCI.GraphicsPipeline.InputLayout.NumElements = 2;

        psoCI.GraphicsPipeline.NumRenderTargets = 1;
        psoCI.GraphicsPipeline.RTVFormats[0] = m_pSwapChain->GetDesc().ColorBufferFormat;
        psoCI.GraphicsPipeline.DSVFormat = m_pSwapChain->GetDesc().DepthBufferFormat;

        psoCI.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        psoCI.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_NONE;
        psoCI.GraphicsPipeline.DepthStencilDesc.DepthEnable = False;

        m_pDevice->CreateGraphicsPipelineState(psoCI, &m_pTrianglePSO);
    }

    void RTGDRenderSystem::CreateTriangleVertexBuffer()
    {
        using namespace Diligent;

        BufferDesc vbDesc;
        vbDesc.Name = "Triangle VB";
        vbDesc.Size = sizeof(kVerts);
        vbDesc.Usage = USAGE_IMMUTABLE;
        vbDesc.BindFlags = BIND_VERTEX_BUFFER;

        BufferData vbData;
        vbData.pData = kVerts;
        vbData.DataSize = sizeof(kVerts);

        m_pDevice->CreateBuffer(vbDesc, &vbData, &m_pTriangleVB);
    }

    void RTGDRenderSystem::CreateShaders()
    {
    }

    void RTGDRenderSystem::Shutdown()
    {
        LogInfo("Render System Shutdown");

        m_pTriangleVB.Release();
        m_pTrianglePSO.Release();
        m_pImmediateContext->Flush();
    }

    void RTGDRenderSystem::BeginFrame()
    {
        using namespace Diligent;

        auto* pRTV = m_pSwapChain->GetCurrentBackBufferRTV();
        auto* pDSV = m_pSwapChain->GetDepthBufferDSV();

        const float clearColor[] = {0.1f, 0.1f, 0.1f, 1.0f};

        m_pImmediateContext->SetRenderTargets(
            1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pImmediateContext->ClearRenderTarget(
            pRTV, clearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pImmediateContext->ClearDepthStencil(
            pDSV, CLEAR_DEPTH_FLAG, 1.0f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        // Triangle, remove later
        m_pImmediateContext->SetPipelineState(m_pTrianglePSO);

        IBuffer* pVBs[] = {m_pTriangleVB};
        Uint64 offsets[] = {0};
        m_pImmediateContext->SetVertexBuffers(
            0, 1, pVBs, offsets,
            RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
            SET_VERTEX_BUFFERS_FLAG_RESET);

        DrawAttribs draw;
        draw.NumVertices = 3;
        draw.Flags = DRAW_FLAG_VERIFY_ALL;
        m_pImmediateContext->Draw(draw);
    }

    void RTGDRenderSystem::EndFrame()
    {
        m_pSwapChain->Present();
    }

    void RTGDRenderSystem::Resize(int width, int height)
    {
        if (!m_pSwapChain)
            return;

        m_width = width;
        m_height = height;
        m_pSwapChain->Resize(width, height);
    }
}
