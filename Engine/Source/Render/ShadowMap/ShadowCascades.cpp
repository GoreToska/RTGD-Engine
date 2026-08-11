//
// Created by ivan on 8/10/26.
//

#include "Render/ShadowMap/ShadowCascades.h"
#include "Tools/CameraFrustum.h"

namespace RTGDEngine {
    void CascadeGrid(const uint32_t cascadeCount, uint32_t &cols, uint32_t &rows) {
        cols = cascadeCount > 1 ? 2 : 1;
        rows = cascadeCount > 2 ? 2 : 1;
    }

    float GetCascadeSplitFar(const float nearZ, const float farZ, const uint32_t index, const float lambda,
                             const uint32_t count) {
        const float p = static_cast<float>(index + 1) / static_cast<float>(count);
        const float logSplit = nearZ * std::pow(farZ / nearZ, p);
        const float uniformSplit = nearZ + (farZ - nearZ) * p;
        return lambda * logSplit + (1.0f - lambda) * uniformSplit;
    }

    CascadeFit BuildCascadeMatrix(const CameraComponent &camera, const TransformComponent &transform,
                                  const Float3 &lightDirection, float sliceNear, float sliceFar, uint32_t resolution) {
        const CameraFrustum slice = CameraFrustum::FromPerspective(
            transform.Position, transform.GetRight(), transform.GetUp(), transform.GetForward(),
            camera.FOVDegrees * Diligent::PI_F / 180.0f, camera.AspectRatio, sliceNear, sliceFar);

        const BoundingSphere bounds = slice.GetBoundingSphere();
        const float radius = std::ceil(bounds.Radius * 16.0f) / 16.0f;
        const float texelWorldSize = 2.0f * radius / static_cast<float>(resolution);
        constexpr float casterPadding = 100.0f;
        const float depthRange = 2.0f * radius + casterPadding;

        const Float3 direction = Diligent::normalize(lightDirection);
        const Float3 up = std::abs(direction.y) > 0.99f ? Float3{0, 0, 1} : Float3{0, 1, 0};
        const Float3 right = Diligent::normalize(Diligent::cross(up, direction));
        const Float3 realUp = Diligent::cross(direction, right);

        const float snappedX = std::floor(Diligent::dot(bounds.Center, right) / texelWorldSize) * texelWorldSize;
        const float snappedY = std::floor(Diligent::dot(bounds.Center, realUp) / texelWorldSize) * texelWorldSize;
        const Float3 center = right * snappedX + realUp * snappedY + direction * Diligent::dot(
                                  bounds.Center, direction);

        const Float3 eye = center - direction * (radius + casterPadding);
        const Matrix4 view = LookAtLH(eye, center, up);
        const Matrix4 projection = Matrix4::OrthoOffCenter(-radius, radius, -radius, radius, 0.0f,
                                                           depthRange, false);


        return {view * projection, depthRange, texelWorldSize};
    }
} // RTGDEngine
