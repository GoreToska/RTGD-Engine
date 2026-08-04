//
// Created by ivan on 8/4/26.
//

#define TINYBVH_IMPLEMENTATION

#include "Tools/SceneBVH.h"

namespace RTGDEngine {
    void SceneBVH::Build(std::span<const AABB> boxes) {
        m_primitiveCount = 0;
        m_packed.clear();

        if (boxes.empty()) return;

        m_packed.reserve(boxes.size() * 2);

        for (const AABB &aabb: boxes) {
            m_packed.emplace_back(aabb.Min.x, aabb.Min.y, aabb.Min.z, 0.0f);
            m_packed.emplace_back(aabb.Max.x, aabb.Max.y, aabb.Max.z, 0.0f);
        }

        m_bvh.BuildAABB(m_packed.data(), static_cast<uint32_t>(boxes.size()));
        m_primitiveCount = static_cast<uint32_t>(boxes.size());
    }

    AABB SceneBVH::ToAABB(const tinybvh::BVH::BVHNode &node) {
        return {
            {node.aabbMin.x, node.aabbMin.y, node.aabbMin.z},
            {node.aabbMax.x, node.aabbMax.y, node.aabbMax.z},
        };
    }
} // RTGDEngine
