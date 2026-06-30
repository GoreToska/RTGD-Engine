//
// Created by gorev on 12.03.2026.
//

#pragma once
#include "BasicMath.hpp"
#include <nlohmann/json.hpp>

using Float3 = Diligent::float3;
using Float2 = Diligent::float2;
using Float4 = Diligent::float4;
using Matrix4 = Diligent::float4x4;
using Quaternion = Diligent::QuaternionF;

constexpr Quaternion QuaternionIdentity() {
    return {0.0f, 0.0f, 0.0f, 1.0f};
}
