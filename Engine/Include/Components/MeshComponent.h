//
// Created by gorev on 12.03.2026.
//

#pragma once
#include "Buffer.h"
#include "RefCntAutoPtr.hpp"
#include "Render/RenderHandle.h"

namespace RTGDEngine
{
    struct MeshComponent
    {
        MeshHandle mesh = INVALID_HANDLE;
        MaterialHandle material = INVALID_HANDLE;
    };
}
