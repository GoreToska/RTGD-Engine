//
// Created by ivan on 7/31/26.
//

#pragma once
#include <cstdint>
#include <array>

#include "Alias.h"
#include "Bounds.h"

namespace RTGDEngine {
    class CameraFrustum {
    public:
        static constexpr uint32_t CORNER_COUNT = 8;
        static constexpr uint32_t PLANE_COUNT = 6;

        CameraFrustum() = default;

        static CameraFrustum FromPerspective(const Float3 &position, const Float3 &right, const Float3 &up,
                                             const Float3 &forward,
                                             float fovYRadians, float aspectRatio, float nearZ, float farZ);

        static CameraFrustum FromViewProjection(const Matrix4 &viewProjection);

        [[nodiscard]] const std::array<Float3, CORNER_COUNT> &GetCorners() const { return m_corners; }

        [[nodiscard]] Float3 GetCenter() const;

        [[nodiscard]] BoundingSphere GetBoundingSphere() const;

        [[nodiscard]] bool Intersects(const BoundingSphere &sphere) const;

        [[nodiscard]] bool Intersects(const AABB &box) const;

        [[nodiscard]] const std::array<Float4, PLANE_COUNT> &GetPlanes() const { return m_planes; }

    private:
        void BuildPlanes();

        static Float4 PlaneFromPoints(const Float3 &a, const Float3 &b, const Float3 &c);

        std::array<Float3, CORNER_COUNT> m_corners = {};
        std::array<Float4, PLANE_COUNT> m_planes = {}; // xyz - normal, w - distance
    };
}
