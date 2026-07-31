//
// Created by ivan on 7/31/26.
//

#include "Tools/CameraFrustum.h"
#include <cmath>
#include <algorithm>

namespace RTGDEngine {
    CameraFrustum CameraFrustum::FromPerspective(const Float3 &position, const Float3 &right, const Float3 &up,
                                                 const Float3 &forward, float fovYRadians, float aspectRatio,
                                                 float nearZ, float farZ) {
        CameraFrustum frustum;

        const float tanV = std::tan(fovYRadians * 0.5f);
        const float tanH = tanV * aspectRatio;

        for (uint32_t f = 0; f < 2; ++f) {
            const float distance = f == 0 ? nearZ : farZ;
            const Float3 center = position + forward * distance;
            const float halfHeight = distance * tanV;
            const float halfWidth = distance * tanH;

            for (uint32_t t = 0; t < 2; ++t) {
                for (uint32_t r = 0; r < 2; ++r) {
                    frustum.m_corners[(f << 2) | (t << 1) | r]
                            = center + right * (r ? halfWidth : -halfWidth)
                              + up * (t ? halfHeight : -halfHeight);
                }
            }
        }

        frustum.BuildPlanes();
        return frustum;
    }

    Float3 CameraFrustum::GetCenter() const {
        Float3 center = {0, 0, 0};
        for (const auto &corner: m_corners) {
            center += corner;
        }

        return center * (1.0f / static_cast<float>(CORNER_COUNT));
    }

    BoundingSphere CameraFrustum::GetBoundingSphere() const {
        BoundingSphere sphere;
        sphere.Center = GetCenter();

        for (const auto &corner: m_corners) {
            sphere.Radius = std::max(sphere.Radius, Diligent::length(corner - sphere.Center));
        }

        return sphere;
    }

    bool CameraFrustum::Intersects(const BoundingSphere &sphere) const {
        for (const auto &plane: m_planes) {
            if (Diligent::dot(Float3{plane.x, plane.y, plane.z}, sphere.Center) + plane.w < -sphere.Radius)
                return false;
        }

        return true;
    }

    bool CameraFrustum::Intersects(const AABB &box) const {
        for (const auto &plane: m_planes) {
            const Float3 positive = {
                plane.x >= 0.0f ? box.Max.x : box.Min.x,
                plane.y >= 0.0f ? box.Max.y : box.Min.y,
                plane.z >= 0.0f ? box.Max.z : box.Min.z
            };

            if (Diligent::dot(Float3{plane.x, plane.y, plane.z}, positive) + plane.w < 0.0f)
                return false;
        }

        return true;
    }

    void CameraFrustum::BuildPlanes() {
        static constexpr uint32_t PLANE_CORNERS[PLANE_COUNT][3] =
        {
            {0, 1, 3}, {4, 5, 7}, {0, 2, 4}, {1, 3, 5}, {0, 1, 4}, {2, 3, 6}
        };

        for (uint32_t i = 0; i < PLANE_COUNT; ++i) {
            m_planes[i] = PlaneFromPoints(m_corners[PLANE_CORNERS[i][0]],
                                          m_corners[PLANE_CORNERS[i][1]],
                                          m_corners[PLANE_CORNERS[i][2]]);
        }

        const Float3 center = GetCenter();
        for (auto &plane: m_planes) {
            if (Diligent::dot(Float3{plane.x, plane.y, plane.z}, center) + plane.w < 0.0f)
                plane = {-plane.x, -plane.y, -plane.z, -plane.w};
        }
    }

    Float4 CameraFrustum::PlaneFromPoints(const Float3 &a, const Float3 &b, const Float3 &c) {
        const Float3 n = Diligent::normalize(Diligent::cross(b - a, c - a));
        return {n.x, n.y, n.z, -Diligent::dot(n, a)};
    }
}
