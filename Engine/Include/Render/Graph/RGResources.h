//
// Created by ivan on 7/15/26.
//

#pragma once
#include <cstdint>
#include <vector>
#include "SwapChain.h"


namespace RTGDEngine {
    struct RGHandle {
        uint32_t ID = ~0u;
        [[nodiscard]] bool IsValid() const { return ID != ~0u; }
    };

    class RGResources {
    public:
        explicit RGResources(Diligent::ISwapChain &swapChain);

        RGHandle ImportTexture(const char *name, Diligent::ITexture *texture);

        RGHandle ImportBackbuffer(const char *name = "Backbuffer");

        RGHandle ImportSwapchainDepth(const char *name = "SwapchainDepth");

        [[nodiscard]] Diligent::ITextureView *RTV(RGHandle h) const;

        [[nodiscard]] Diligent::ITextureView *SRV(RGHandle h) const;

        [[nodiscard]] Diligent::ITextureView *DSV(RGHandle h) const;

        [[nodiscard]] RGHandle Find(const char *name) const;

    private:
        enum class Kind {
            Texture,
            Backbuffer,
            SwapchainDepth,
        };

        struct Entry {
            const char *Name;
            Kind kind;
            Diligent::ITexture *Texture = nullptr; // only for kind == Texture
        };

        [[nodiscard]] const Entry *Get(RGHandle handle) const;

        Diligent::ISwapChain *m_swapChain = nullptr;
        std::vector<Entry> m_entries = {};
    };
} // RTGDEngine
