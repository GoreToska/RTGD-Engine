//
// Created by gorev on 12.03.2026.
//

#pragma once
#include <cstdint>

namespace RTGDEngine {
    using MeshHandle = uint32_t;
    using MaterialHandle = uint32_t;

    using TextureHandle = uint32_t;

    constexpr uint32_t INVALID_TEXTURE_HANDLE = UINT32_MAX;
    constexpr uint32_t INVALID_MESH_HANDLE = UINT32_MAX;
    constexpr uint32_t INVALID_MATERIAL_HANDLE = UINT32_MAX;
}
