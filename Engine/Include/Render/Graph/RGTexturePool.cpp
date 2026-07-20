//
// Created by ivan on 7/20/26.
//

#include "RGTexturePool.h"

#include "Tools/Logger.h"

namespace RTGDEngine {
    Diligent::ITexture *RGTexturePool::Acquire(Diligent::IRenderDevice &device, const RGTextureDesc &desc,
                                               uint32_t fullWidth, uint32_t fullHeight) {
        using namespace Diligent;

        const uint32_t width = desc.Width ? desc.Width : fullWidth;
        const uint32_t height = desc.Height ? desc.Height : fullHeight;

        if (auto it = m_entries.find(desc.Name); it != m_entries.end()) {
            const Entry &e = it->second;
            if (e.Width == width && e.Height == height && e.Format == desc.Format && e.BindFlags == desc.BindFlags) {
                return e.Texture;
            }
        }

        TextureDesc td = {};
        td.Name = desc.Name;
        td.Type = RESOURCE_DIM_TEX_2D;
        td.Width = width;
        td.Height = height;
        td.MipLevels = 1;
        td.Format = desc.Format;
        td.BindFlags = desc.BindFlags;
        td.Usage = USAGE_DEFAULT;

        RefCntAutoPtr<ITexture> tex;
        device.CreateTexture(td, nullptr, &tex);

        Entry &e = m_entries[desc.Name];
        e.Texture = tex;
        e.Width = width;
        e.Height = height;
        e.Format = desc.Format;
        e.BindFlags = desc.BindFlags;

        LogInfo("Created texture '{}', {}x{}", desc.Name, width, height);
        return e.Texture;
    }

    void RGTexturePool::Invalidate() {
        m_entries.clear();
    }
} // RTGDEngine
