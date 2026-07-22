//
// Created by ivan on 7/16/26.
//

#pragma once
#include <GraphicsTypes.h>
#include "RGResources.h"


namespace RTGDEngine {
    enum class RGAccess {
        RenderTarget,
        DepthWrite,
        ShaderResource,
    };

    inline Diligent::RESOURCE_STATE ToResourceState(RGAccess access) {
        switch (access) {
            case RGAccess::RenderTarget:
                return Diligent::RESOURCE_STATE_RENDER_TARGET;
            case RGAccess::DepthWrite:
                return Diligent::RESOURCE_STATE_DEPTH_WRITE;
            case RGAccess::ShaderResource:
                return Diligent::RESOURCE_STATE_SHADER_RESOURCE;
        }

        return Diligent::RESOURCE_STATE_UNKNOWN;
    }

    struct RGAccessDecl {
        RGHandle Handle;
        RGAccess Access;
    };

    class RGBuilder {
    public:
        RGBuilder(RGResources &res, std::vector<RGAccessDecl> &io)
            : m_resources(res), m_io(io) {
        };

        RGHandle Read(const char *name) {
            return Push(m_resources.Find(name), RGAccess::ShaderResource);
        }

        RGHandle WriteColor(const char *name) {
            return Push(m_resources.Find(name), RGAccess::RenderTarget);
        }

        RGHandle WriteDepth(const char *name) {
            return Push(m_resources.Find(name), RGAccess::DepthWrite);
        }

        RGHandle CreateColor(const RGTextureDesc &desc) {
            return Push(m_resources.CreateColor(desc), RGAccess::RenderTarget);
        }

    private:
        RGHandle Push(RGHandle handle, RGAccess access) {
            m_io.push_back({handle, access});
            return handle;
        }

        RGResources &m_resources;
        std::vector<RGAccessDecl> &m_io;
    };
}
