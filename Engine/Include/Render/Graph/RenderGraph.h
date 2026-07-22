//
// Created by ivan on 7/14/26.
//

#pragma once
#include <memory>
#include <vector>

#include "Pass/IRenderPass.h"
#include <SwapChain.h>
#include <RenderDevice.h>

#include "RGTexturePool.h"

namespace RTGDEngine {
    struct RenderContext;

    class RenderGraph {
    public:
        void AddPass(std::unique_ptr<IRenderPass> pass);

        void Initialize(Diligent::IRenderDevice &device, Diligent::ISwapChain &swapChain) const;


        void Execute(RenderContext &context);

        void InvalidateTransientResources();

        Diligent::ITexture *FindTexture(const std::string &name);

    private:
        std::vector<std::unique_ptr<IRenderPass> > m_passes = {};
        RGTexturePool m_texturePool = {};

        void SetupPasses(RenderContext &context);
    };
} // RTGDEngine
