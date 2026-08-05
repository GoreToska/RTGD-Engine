//
// Created by ivan on 7/31/26.
//

#pragma once
#include "Alias.h"

namespace RTGDEngine {
    struct BoundingSphere {
        Float3 Center = {0, 0, 0};
        float Radius = 0.0f;
    };

    struct AABB {
        Float3 Min = {0, 0, 0};
        Float3 Max = {0, 0, 0};

        void SetFromExtent(const Float3 center, const Float3 extent) {
            Min = center - extent;
            Max = center + extent;
        }

        [[nodiscard]] Float3 Center() const { return (Min + Max) * 0.5f; }
        [[nodiscard]] Float3 Extent() const { return (Max - Min) * 0.5f; }
        [[nodiscard]] bool IsValid() const { return Max.x >= Min.x && Max.y >= Min.y && Max.z >= Min.z; }
    };

    [[nodiscard]] inline AABB TransformBounds(const AABB &bounds, const Matrix4 &m) {
        const Float3 c = bounds.Center();
        const Float3 r = bounds.Extent();
        auto worldCenter = c * m;

        const Float3 worldExtent = {
            std::abs(m.m00) * r.x + std::abs(m.m10) * r.y + std::abs(m.m20) * r.z,
            std::abs(m.m01) * r.x + std::abs(m.m11) * r.y + std::abs(m.m21) * r.z,
            std::abs(m.m02) * r.x + std::abs(m.m12) * r.y + std::abs(m.m22) * r.z
        };

        AABB out;
        out.SetFromExtent(worldCenter, worldExtent);
        return out;
    }
}
