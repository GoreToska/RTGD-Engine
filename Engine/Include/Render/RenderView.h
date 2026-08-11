//
// Created by ivan on 8/6/26.
//

#pragma once
#include "Tools/CameraFrustum.h"
#include "Tools/Visibility.h"

namespace RTGDEngine {
    struct RenderView {
        CameraFrustum Frustum;
        Matrix4 ViewProjection = Matrix4::Identity();
        VisibilityMask Mask;
        float DepthRange = 1.0f; // only for shadows
        float TexelWorldSize = 1.0f; // only for shadows
        float SplitFar = 0.0f; // only for shadows
    };
}
