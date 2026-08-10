//
// Created by ivan on 8/6/26.
//

#pragma once
#include "Tools/CameraFrustum.h"
#include "Tools/Visibility.h"

namespace RTGDEngine {
    struct RenderView {
        CameraFrustum Frustum;
        VisibilityMask Mask;
    };
}
