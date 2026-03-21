//
// Created by gorev on 17.03.2026.
//

#pragma once
#include "RefCntAutoPtr.hpp"
#include "Texture.h"
#include "Engine/EngineExport.h"

namespace RTGDEngine
{
    struct ENGINE_API GBuffer
    {
        Diligent::RefCntAutoPtr<Diligent::ITexture> DiffuseTexture;
        Diligent::RefCntAutoPtr<Diligent::ITexture> NormalTexture;
        Diligent::RefCntAutoPtr<Diligent::ITexture> PositionTexture;

        Diligent::RefCntAutoPtr<Diligent::ITextureView> DiffuseRTV;
        Diligent::RefCntAutoPtr<Diligent::ITextureView> NormalRTV;
        Diligent::RefCntAutoPtr<Diligent::ITextureView> PositionRTV;

        // Metallic (R), Roughness(G), AO (B)
        Diligent::RefCntAutoPtr<Diligent::ITexture> PBRTexture;
        Diligent::RefCntAutoPtr<Diligent::ITextureView> PBRRTV;
        Diligent::RefCntAutoPtr<Diligent::ITextureView> PBRSRV;

        Diligent::RefCntAutoPtr<Diligent::ITextureView> DiffuseSRV;
        Diligent::RefCntAutoPtr<Diligent::ITextureView> NormalSRV;
        Diligent::RefCntAutoPtr<Diligent::ITextureView> PositionSRV;

        Diligent::RefCntAutoPtr<Diligent::ITexture> DepthTexture;
        Diligent::RefCntAutoPtr<Diligent::ITextureView> DepthDSV;
        Diligent::RefCntAutoPtr<Diligent::ITextureView> DepthSRV;

        uint32_t Width = 0;
        uint32_t Height = 0;

        bool IsValid() const { return DiffuseRTV && NormalRTV && PositionRTV; }
    };
} // RTGDEngine
