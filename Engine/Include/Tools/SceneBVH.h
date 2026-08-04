//
// Created by ivan on 8/4/26.
//

#pragma once
#include <cstdint>
#include <span>
#include <vector>

#include <tiny_bvh.h>

#include "Bounds.h"
#include "CameraFrustum.h"

namespace RTGDEngine {
    class SceneBVH {
    public:
        SceneBVH() = default;

        SceneBVH(const SceneBVH &) = delete;

        SceneBVH &operator=(const SceneBVH &) = delete;

        void Build(std::span<const AABB> boxes);

        void Clear() { m_primitiveCount = 0; }

        [[nodiscard]] bool IsEmpty() const { return m_primitiveCount == 0; }

        [[nodiscard]] uint32_t GetPrimitiveCount() const { return m_primitiveCount; }

        template<typename Fn>
        void Query(const CameraFrustum &frustum, Fn &&onVisible) const {
            if (m_primitiveCount == 0)
                return;

            uint32_t stack[MAX_STACK_DEPTH];
            uint32_t stackPtr = 0;
            stack[stackPtr++] = 0;

            while (stackPtr > 0) {
                const uint32_t nodeIndex = stack[--stackPtr];
                const tinybvh::BVH::BVHNode &node = m_bvh.bvhNode[nodeIndex];

                const CameraFrustum::Containment containment = frustum.Classify(ToAABB(node));

                if (containment == CameraFrustum::Containment::Outside)
                    continue;

                if (containment == CameraFrustum::Containment::Inside) {
                    EmitSubtree(nodeIndex, onVisible);
                    continue;
                }

                if (node.isLeaf()) {
                    EmitLeaf(node, onVisible);
                    continue;
                }

                stack[stackPtr++] = node.leftFirst;
                stack[stackPtr++] = node.leftFirst + 1;
            }
        }

    private:
        static constexpr uint32_t MAX_STACK_DEPTH = 64;

        static AABB ToAABB(const tinybvh::BVH::BVHNode &node);

        template<typename Fn>
        void EmitLeaf(const tinybvh::BVH::BVHNode &node, Fn &&onVisible) const {
            for (uint32_t i = 0; i < node.triCount; ++i)
                onVisible(m_bvh.primIdx[node.leftFirst + i]);
        }

        template<typename Fn>
        void EmitSubtree(uint32_t rootIndex, Fn &&onVisible) const {
            uint32_t stack[MAX_STACK_DEPTH];
            uint32_t stackPtr = 0;
            stack[stackPtr++] = rootIndex;

            while (stackPtr > 0) {
                const tinybvh::BVH::BVHNode &node = m_bvh.bvhNode[stack[--stackPtr]];

                if (node.isLeaf()) {
                    EmitLeaf(node, onVisible);
                    continue;
                }

                stack[stackPtr++] = node.leftFirst;
                stack[stackPtr++] = node.leftFirst + 1;
            }
        }

        tinybvh::BVH m_bvh;
        std::vector<tinybvh::bvhvec4> m_packed;
        uint32_t m_primitiveCount = 0;
    };
} // RTGDEngine
