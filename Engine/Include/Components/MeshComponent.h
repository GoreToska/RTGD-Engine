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

        static void RegisterMeta(const flecs::world& world)
        {
            flecs::component<MeshComponent>(world, "MeshComponent")
                    .member<MeshHandle>("MeshHandle")
                    .member<MaterialHandle>("MaterialHandle");
        }
    };
}
