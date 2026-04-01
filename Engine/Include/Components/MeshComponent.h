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
        MeshHandle meshHandle = INVALID_MESH_HANDLE;
        MaterialHandle materialHandle = INVALID_MESH_HANDLE;
    };
}
