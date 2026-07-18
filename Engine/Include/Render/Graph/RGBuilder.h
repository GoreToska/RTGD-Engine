//
// Created by ivan on 7/16/26.
//

#pragma once
#include "RGResources.h"

namespace RTGDEngine {
    enum class RGAccess {
        RenderTarget,
        DepthWrite,
        ShaderResource,
    };

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

    private:
        RGHandle Push(RGHandle handle, RGAccess access) {
            m_io.push_back({handle, access});
            return handle;
        }

        RGResources &m_resources;
        std::vector<RGAccessDecl> &m_io;
    };
}
