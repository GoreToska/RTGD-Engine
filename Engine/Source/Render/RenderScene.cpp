//
// Created by ivan on 8/6/26.
//

#include "Render/RenderScene.h"

#include "Components/MeshComponent.h"
#include "Components/RenderComponent.h"
#include "Components/TransformComponent.h"

namespace RTGDEngine {
    void RenderScene::Gather(flecs::world &world) {
        m_centerX.clear();
        m_centerY.clear();
        m_centerZ.clear();
        m_extentX.clear();
        m_extentY.clear();
        m_extentZ.clear();
        m_world.clear();
        m_mesh.clear();
        m_material.clear();
        m_flags.clear();
        m_count = 0;

        const int upper = world.count<MeshComponent>();

        m_centerX.reserve(upper);
        m_centerY.reserve(upper);
        m_centerZ.reserve(upper);
        m_extentX.reserve(upper);
        m_extentY.reserve(upper);
        m_extentZ.reserve(upper);
        m_world.reserve(upper);
        m_mesh.reserve(upper);
        m_material.reserve(upper);
        m_flags.reserve(upper);

#ifdef RTGD_EDITOR
        m_entities.clear();
        m_entities.reserve(upper);
#endif

        auto &rm = RenderResourceManager::Instance();

        world.each([&](flecs::entity e, const MeshComponent &mc, const RenderComponent &rc, TransformComponent &tc) {
            if (!rc.IsVisible) return;

            if (!mc.Mesh.Handle.IsValid()) return;

            const MeshData &md = rm.GetMesh(mc.Mesh.Handle);
            if (!md.VertexBuffer) return;

            const Matrix4 worldMatrix = tc.GetWorldMatrix();
            const AABB box = md.LocalBounds.IsValid() ? TransformBounds(md.LocalBounds, worldMatrix) : AABB{};
            const uint8_t flags = static_cast<uint8_t>((rc.CastShadows ? 1 : 0) | (md.LocalBounds.IsValid() ? 0 : 2));
            Add(box, worldMatrix, mc.Mesh.Handle, mc.Material.Handle, flags, e);
        });

        m_count = static_cast<uint32_t>(m_centerX.size());
        const size_t padded = ((m_count + 63) / 64) * 64;
        m_centerX.resize(padded, 0.0f);
        m_centerY.resize(padded, 0.0f);
        m_centerZ.resize(padded, 0.0f);
        m_extentX.resize(padded, 0.0f);
        m_extentY.resize(padded, 0.0f);
        m_extentZ.resize(padded, 0.0f);

        m_shadowCasters.Resize(m_count);
        m_alwaysVisible.Resize(m_count);

        for (uint32_t i = 0; i < m_count; ++i) {
            if (m_flags[i] & 1) m_shadowCasters.Set(i);
            if (m_flags[i] & 2) m_alwaysVisible.Set(i);
        }
    }

    void RenderScene::Add(const AABB &box, const Matrix4 &world, MeshHandle mesh, MaterialHandle material,
                                      uint8_t flags, flecs::entity entity) {
        m_centerX.push_back(box.Center().x);
        m_centerY.push_back(box.Center().y);
        m_centerZ.push_back(box.Center().z);
        m_extentX.push_back(box.Extent().x);
        m_extentY.push_back(box.Extent().y);
        m_extentZ.push_back(box.Extent().z);
        m_world.push_back(world);
        m_mesh.push_back(mesh);
        m_material.push_back(material);
        m_flags.push_back(flags);

#ifdef RTGD_EDITOR
        m_entities.push_back(entity);
#endif
    }

    BoundsView RenderScene::Bounds() const {
        auto bounds = BoundsView();
        bounds.CenterX = m_centerX.data();
        bounds.CenterY = m_centerY.data();
        bounds.CenterZ = m_centerZ.data();
        bounds.ExtentX = m_extentX.data();
        bounds.ExtentY = m_extentY.data();
        bounds.ExtentZ = m_extentZ.data();
        bounds.Count = m_count;
        return bounds;
    }

    uint32_t RenderScene::Count() const {
        return m_count;
    }

    const Matrix4 *RenderScene::World() const {
        return m_world.data();
    }

    const MeshHandle *RenderScene::Mesh() const {
        return m_mesh.data();
    }

    const MaterialHandle *RenderScene::Material() const {
        return m_material.data();
    }

    const VisibilityMask &RenderScene::ShadowCasters() const {
        return m_shadowCasters;
    }

    const VisibilityMask &RenderScene::AlwaysVisible() const {
        return m_alwaysVisible;
    }

#ifdef RTGD_EDITOR
    const std::vector<flecs::entity> &RenderScene::Entities() const {
        return m_entities;
    }
#endif
}
