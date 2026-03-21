//
// Created by gorev on 17.03.2026.
//

#include "Render/GBufferFactory.h"

#include "RenderDevice.h"
#include "Tools/Logger.h"

namespace RTGDEngine
{
    static Diligent::RefCntAutoPtr<Diligent::ITexture> CreateRenderTarget(
        Diligent::IRenderDevice& device,
        const char* name,
        uint32_t width,
        uint32_t height,
        Diligent::TEXTURE_FORMAT format)
    {
        using namespace Diligent;

        TextureDesc desc;
        desc.Name = name;
        desc.Type = RESOURCE_DIM_TEX_2D;
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.Format = format;
        desc.BindFlags = BIND_RENDER_TARGET | BIND_SHADER_RESOURCE;
        desc.Usage = USAGE_DEFAULT;

        RefCntAutoPtr<ITexture> texture;
        device.CreateTexture(desc, nullptr, &texture);
        return texture;
    }

    GBuffer GBufferFactory::Create(Diligent::IRenderDevice& device, uint32_t width, uint32_t height)
    {
        using namespace Diligent;

        GBuffer gbuffer;
        gbuffer.Width = width;
        gbuffer.Height = height;

        gbuffer.DiffuseTexture = CreateRenderTarget(
            device, "GBuffer Diffuse", width, height,
            TEX_FORMAT_RGBA8_UNORM_SRGB);

        gbuffer.NormalTexture = CreateRenderTarget(
            device, "GBuffer Normal", width, height,
            TEX_FORMAT_RGBA16_FLOAT);

        gbuffer.PositionTexture = CreateRenderTarget(
            device, "GBuffer Position", width, height,
            TEX_FORMAT_RGBA32_FLOAT);

        gbuffer.PBRTexture = CreateRenderTarget(
            device, "GBuffer PBR", width, height,
            TEX_FORMAT_RGBA8_UNORM);

        TextureDesc depthDesc;
        depthDesc.Name = "GBuffer Depth";
        depthDesc.Type = RESOURCE_DIM_TEX_2D;
        depthDesc.Width = width;
        depthDesc.Height = height;
        depthDesc.MipLevels = 1;
        depthDesc.Format = TEX_FORMAT_D32_FLOAT;
        depthDesc.BindFlags = BIND_DEPTH_STENCIL | BIND_SHADER_RESOURCE;
        depthDesc.Usage = USAGE_DEFAULT;
        device.CreateTexture(depthDesc, nullptr, &gbuffer.DepthTexture);

        gbuffer.DiffuseRTV = gbuffer.DiffuseTexture->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
        gbuffer.NormalRTV = gbuffer.NormalTexture->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
        gbuffer.PositionRTV = gbuffer.PositionTexture->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
        gbuffer.DepthDSV = gbuffer.DepthTexture->GetDefaultView(TEXTURE_VIEW_DEPTH_STENCIL);
        gbuffer.PBRRTV = gbuffer.PBRTexture->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);

        gbuffer.DiffuseSRV = gbuffer.DiffuseTexture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
        gbuffer.NormalSRV = gbuffer.NormalTexture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
        gbuffer.PositionSRV = gbuffer.PositionTexture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
        gbuffer.DepthSRV = gbuffer.DepthTexture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
        gbuffer.PBRSRV = gbuffer.PBRTexture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

        LogInfo("GBuffer created: {}x{}", width, height);
        return gbuffer;
    }

    void GBufferFactory::Resize(GBuffer& gbuffer, Diligent::IRenderDevice& device, uint32_t width, uint32_t height)
    {
        if (gbuffer.Width == width && gbuffer.Height == height)
            return;

        gbuffer = Create(device, width, height);
        LogInfo("GBuffer resized: {}x{}", width, height);
    }
} // RTGDEngine
