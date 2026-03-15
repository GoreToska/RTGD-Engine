//
// Created by gorev on 13.03.2026.
//

#pragma once
#include "Tools/Alias.h"

namespace RTGDEngine
{
    struct alignas(16) CameraConstantBuffer
    {
        Matrix4 View;
        Matrix4 Projection;
        Float4 CameraPosition;
    };

    struct alignas(16) ObjectConstantBuffer
    {
        Matrix4 Model;
    };
}
