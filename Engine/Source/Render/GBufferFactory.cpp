//
// Created by gorev on 17.03.2026.
//

#include "Render/GBufferFactory.h"

#include "RenderDevice.h"
#include "Tools/Logger.h"

namespace RTGDEngine {
    static Diligent::RefCntAutoPtr<Diligent::ITexture> CreateRenderTarget(
        Diligent::IRenderDevice &device,
        const char *name,
        uint32_t width,
        uint32_t height,
        Diligent::TEXTURE_FORMAT format) {
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

    GBuffer GBufferFactory::Create(Diligent::IRenderDevice &device, uint32_t width, uint32_t height) {
        using namespace Diligent;

        GBuffer gbuffer;
        gbuffer.Width = width;
        gbuffer.Height = height;

#ifdef RTGD_EDITOR
        gbuffer.IDTexture = CreateRenderTarget(
            device, "GBuffer Entity ID", width, height,
            TEX_FORMAT_R32_UINT);

        gbuffer.IDRTV = gbuffer.IDTexture->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
        gbuffer.IDSRV = gbuffer.IDTexture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

        TextureDesc idReadbackDesc;
        idReadbackDesc.Name = "GBuffer ID Readback";
        idReadbackDesc.Type = RESOURCE_DIM_TEX_2D;
        idReadbackDesc.Width = 1;
        idReadbackDesc.Height = 1;
        idReadbackDesc.MipLevels = 1;
        idReadbackDesc.Format = TEX_FORMAT_R32_UINT;
        idReadbackDesc.BindFlags = BIND_NONE;
        idReadbackDesc.Usage = USAGE_STAGING;
        idReadbackDesc.CPUAccessFlags = CPU_ACCESS_READ;
        device.CreateTexture(idReadbackDesc, nullptr, &gbuffer.IDReadbackTexture);
#endif

        LogInfo("GBuffer created: {}x{}", width, height);
        return gbuffer;
    }

    void GBufferFactory::Resize(GBuffer &gbuffer, Diligent::IRenderDevice &device, uint32_t width, uint32_t height) {
        if (gbuffer.Width == width && gbuffer.Height == height)
            return;

        gbuffer = Create(device, width, height);
        LogInfo("GBuffer resized: {}x{}", width, height);
    }
} // RTGDEngine
