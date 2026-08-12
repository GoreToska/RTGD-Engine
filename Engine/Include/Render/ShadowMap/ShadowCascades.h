//
// Created by ivan on 8/10/26.
//

#pragma once
#include "Components/CameraComponent.h"
#include "Components/TransformComponent.h"
#include "Tools/Alias.h"

namespace RTGDEngine {
    struct CascadeFit {
        Matrix4 ViewProjection = Matrix4::Identity();
        float DepthRange = 1;
        float TexelWorldSize = 1;
    };

    void CascadeGrid(const uint32_t cascadeCount, uint32_t &cols, uint32_t &rows);

    float GetCascadeSplitFar(float nearZ, float farZ, uint32_t index, float lambda, uint32_t count);

    CascadeFit BuildCascadeMatrix(const CameraComponent &camera, const TransformComponent &transform,
                                  const Float3 &lightDirection, float sliceNear, float sliceFar,
                                  uint32_t resolution);
} // RTGDEngine
