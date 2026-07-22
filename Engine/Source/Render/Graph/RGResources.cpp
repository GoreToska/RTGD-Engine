//
// Created by ivan on 7/15/26.
//

#include "Render/Graph/RGResources.h"

#include <cstring>

#include "Texture.h"

namespace RTGDEngine {
    RGResources::RGResources(Diligent::ISwapChain &swapChain) : m_swapChain(&swapChain) {
    }

    RGHandle RGResources::ImportTexture(const char *name, Diligent::ITexture *texture) {
        RGHandle h{static_cast<uint32_t>(m_entries.size())};
        m_entries.push_back({name, Kind::Texture, texture});
        return h;
    }

    RGHandle RGResources::ImportBackbuffer(const char *name) {
        RGHandle h{static_cast<uint32_t>(m_entries.size())};
        m_entries.push_back({name, Kind::Backbuffer, nullptr});
        return h;
    }

    RGHandle RGResources::ImportSwapchainDepth(const char *name) {
        RGHandle h{static_cast<uint32_t>(m_entries.size())};
        m_entries.push_back({name, Kind::SwapchainDepth, nullptr});
        return h;
    }

    Diligent::ITextureView *RGResources::RTV(RGHandle h) const {
        const Entry *e = Get(h);
        if (!e) return nullptr;

        switch (e->kind) {
            case Kind::Backbuffer:
                return m_swapChain->GetCurrentBackBufferRTV();
            case Kind::Texture:
            case Kind::Transient:
                return e->Texture ? e->Texture->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET) : nullptr;
            default:
                return nullptr;
        }
    }

    Diligent::ITextureView *RGResources::SRV(RGHandle h) const {
        const Entry *e = Get(h);
        if (!e || (e->kind != Kind::Texture && e->kind != Kind::Transient) || !e->Texture) return nullptr;
        return e->Texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
    }

    Diligent::ITextureView *RGResources::DSV(RGHandle h) const {
        const Entry *e = Get(h);
        if (!e) return nullptr;

        switch (e->kind) {
            case Kind::SwapchainDepth:
                return m_swapChain->GetDepthBufferDSV();
            case Kind::Texture:
            case Kind::Transient:
                return e->Texture ? e->Texture->GetDefaultView(Diligent::TEXTURE_VIEW_DEPTH_STENCIL) : nullptr;
            default:
                return nullptr;
        }
    }

    RGHandle RGResources::Find(const char *name) const {
        for (uint32_t i = 0; i < m_entries.size(); ++i) {
            if (std::strcmp(m_entries[i].Name, name) == 0)
                return RGHandle{i};
        }

        return {};
    }

    Diligent::ITexture *RGResources::Texture(RGHandle handle) const {
        const Entry *e = Get(handle);
        if (!e) return nullptr;
        switch (e->kind) {
            case Kind::Texture:
            case Kind::Transient:
                return e->Texture;
            case Kind::Backbuffer:
                return m_swapChain->GetCurrentBackBufferRTV()->GetTexture();
            case Kind::SwapchainDepth:
                return m_swapChain->GetDepthBufferDSV()->GetTexture();
        }

        return nullptr;
    }

    const char *RGResources::Name(RGHandle handle) const {
        const Entry *e = Get(handle);
        return e ? e->Name : "Invalid!";
    }

    RGHandle RGResources::CreateColor(const RGTextureDesc &desc) {
        RGHandle h{static_cast<uint32_t>(m_entries.size())};
        m_entries.push_back({desc.Name, Kind::Transient, nullptr, desc});
        return h;
    }

    void RGResources::ResolveTransientResources(RGTexturePool &pool, Diligent::IRenderDevice &device) {
        const auto &scDesc = m_swapChain->GetDesc();
        for (Entry &e: m_entries) {
            if (e.kind == Kind::Transient && !e.Texture)
                e.Texture = pool.Acquire(device, e.Desc, scDesc.Width, scDesc.Height);
        }
    }

    const RGResources::Entry *RGResources::Get(RGHandle handle) const {
        if (!handle.IsValid() || handle.ID >= m_entries.size()) return nullptr;
        return &m_entries[handle.ID];
    }
} // RTGDEngine
