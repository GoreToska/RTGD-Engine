//
// Created by ivan on 8/6/26.
//

#pragma once
#include <flecs.h>

#include "RenderHandle.h"
#include "Tools/Visibility.h"

namespace RTGDEngine {
    class RenderScene {
    public:
        void Gather(flecs::world &world);

        void Add(const AABB &box, const Matrix4 &world, MeshHandle mesh, MaterialHandle material, uint8_t flags,
                 flecs::entity entity);

        [[nodiscard]] BoundsView Bounds() const;

        [[nodiscard]] uint32_t Count() const;

        [[nodiscard]] const Matrix4 *World() const;

        [[nodiscard]] const MeshHandle *Mesh() const;

        [[nodiscard]] const MaterialHandle *Material() const;

        [[nodiscard]] const VisibilityMask &ShadowCasters() const;

        [[nodiscard]] const VisibilityMask &AlwaysVisible() const;

#ifdef RTGD_EDITOR
        [[nodiscard]] const std::vector<flecs::entity> &Entities() const;
#endif

    private:
        std::vector<float> m_centerX, m_centerY, m_centerZ;
        std::vector<float> m_extentX, m_extentY, m_extentZ;
        std::vector<Matrix4> m_world;
        std::vector<MeshHandle> m_mesh;
        std::vector<MaterialHandle> m_material;
        VisibilityMask m_shadowCasters;
        VisibilityMask m_alwaysVisible;
        std::vector<uint8_t> m_flags; // 1 - cast shadows, 2 - always visible
        uint32_t m_count = 0;
#ifdef RTGD_EDITOR
        std::vector<flecs::entity> m_entities;
#endif
    };
}
