//
// Created by ivan on 7/20/26.
//

#pragma once
#include <cstdint>

#include <GraphicsTypes.h>
#include <unordered_map>
#include <string>

#include "RefCntAutoPtr.hpp"
#include "RenderDevice.h"
#include "Texture.h"

namespace RTGDEngine {
    struct RGTextureDesc {
        const char *Name;
        uint32_t Width = 0; // 0 = full render resolution
        uint32_t Height = 0; // 0 = full render resolution
        Diligent::TEXTURE_FORMAT Format;
        Diligent::BIND_FLAGS BindFlags = Diligent::BIND_RENDER_TARGET | Diligent::BIND_SHADER_RESOURCE;
    };

    class RGTexturePool {
    public:
        Diligent::ITexture *Acquire(Diligent::IRenderDevice &device, const RGTextureDesc &desc, uint32_t fullWidth,
                                    uint32_t fullHeight);

        // Drop all (on resize for example)
        // This can be a problem on resize
        void Invalidate();

        Diligent::ITexture *Find(const std::string &name);

    private:
        struct Entry {
            Diligent::RefCntAutoPtr<Diligent::ITexture> Texture;
            uint32_t Width;
            uint32_t Height;
            Diligent::TEXTURE_FORMAT Format;
            Diligent::BIND_FLAGS BindFlags;
        };

        std::unordered_map<std::string, Entry> m_entries{};
    };
} // RTGDEngine
