//
// Created by gorev on 17.03.2026.
//

#pragma once
#include "RefCntAutoPtr.hpp"
#include "Texture.h"
#include "Engine/EngineExport.h"

namespace RTGDEngine {
    struct ENGINE_API GBuffer {
        Diligent::RefCntAutoPtr<Diligent::ITexture> DepthTexture;
        Diligent::RefCntAutoPtr<Diligent::ITextureView> DepthDSV;
        Diligent::RefCntAutoPtr<Diligent::ITextureView> DepthSRV;

        uint32_t Width = 0;
        uint32_t Height = 0;

        [[nodiscard]] bool IsValid() const {
#if defined(RTGD_EDITOR)
            return DepthDSV && IDRTV;
#else
            return DepthDSV;
#endif
        }

#ifdef RTGD_EDITOR
        Diligent::RefCntAutoPtr<Diligent::ITexture> IDTexture;
        Diligent::RefCntAutoPtr<Diligent::ITextureView> IDRTV;
        Diligent::RefCntAutoPtr<Diligent::ITextureView> IDSRV;
        Diligent::RefCntAutoPtr<Diligent::ITexture> IDReadbackTexture;
#endif
    };
} // RTGDEngine
