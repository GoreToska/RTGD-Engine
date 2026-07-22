//
// Created by ivan on 7/14/26.
//

#include "Render/Graph/RenderGraph.h"

#include "Render/Graph/RenderContext.h"
#include "Render/Graph/Pass/IRenderPass.h"
#include "Tools/Logger.h"

namespace RTGDEngine {
    void RenderGraph::AddPass(std::unique_ptr<IRenderPass> pass) {
        m_passes.push_back(std::move(pass));
    }

    void RenderGraph::Initialize(Diligent::IRenderDevice &device, Diligent::ISwapChain &swapChain,
                                 GBuffer &gBuffer) const {
        for (const auto &pass: m_passes) {
            pass->Initialize(device, swapChain, gBuffer);
        }
    }

    void RenderGraph::SetupPasses(RenderContext &context) {
        for (const auto &pass: m_passes) {
            if (!pass->IsEnabled()) continue;

            pass->IO().clear();
            RGBuilder builder = RGBuilder(*context.Graph, pass->IO());
            pass->Setup(builder);
        }
    }

    void RenderGraph::Execute(RenderContext &context) {
        SetupPasses(context);

        context.Graph->ResolveTransientResources(m_texturePool, context.Device);

        std::vector<uint32_t> produced{};

        for (const auto &pass: m_passes) {
            if (!pass->IsEnabled()) continue;

            std::vector<Diligent::StateTransitionDesc> barriers{};
            barriers.reserve(m_passes.size());

            for (const auto &io: pass->IO()) {
                if (!io.Handle.IsValid()) {
                    LogError("Pass '{}' referencing unknown resource.", pass->Name());
                    continue;
                }

                if (io.Access == RGAccess::ShaderResource &&
                    std::ranges::find(produced, io.Handle.ID) == produced.end()) {
                    LogWarn("Pass '{}' reads '{}', that was not produced in this frame.", pass->Name(),
                            context.Graph->Name(io.Handle));
                }

                if (auto *tex = context.Graph->Texture(io.Handle)) {
                    barriers.push_back({
                        tex, Diligent::RESOURCE_STATE_UNKNOWN, ToResourceState(io.Access),
                        Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE
                    });
                }
            }

            if (!barriers.empty()) {
                context.Context.TransitionResourceStates(static_cast<Diligent::Uint32>(barriers.size()),
                                                         barriers.data());
            }

            pass->Execute(context);

            for (const auto &io: pass->IO()) {
                if (io.Access != RGAccess::ShaderResource && io.Handle.IsValid()) {
                    produced.push_back(io.Handle.ID);
                }
            }
        }
    }

    void RenderGraph::InvalidateTransientResources() {
        m_texturePool.Invalidate();
    }
} // RTGDEngine
