//
// Created by gorev on 12.03.2026.
//

#pragma once
#include "BasicMath.hpp"
#include <flecs.h>

#include "gainput/gainput.h"

using Float3 = Diligent::float3;
using Float2 = Diligent::float2;
using Float4 = Diligent::float4;
using Matrix4 = Diligent::float4x4;
using Quaternion = Diligent::QuaternionF;

using ActionID = gainput::UserButtonId;
using Entity = flecs::entity;
using World = flecs::world;

constexpr Quaternion QuaternionIdentity()
{
    return {0.0f, 0.0f, 0.0f, 1.0f};
}

inline Matrix4 LookAtLH(const Float3& Eye, const Float3& Target, const Float3& Up)
{
    const Float3 f = normalize(Target - Eye);
    const Float3 r = normalize(cross(Up, f));
    const Float3 u = cross(f, r);
    return Matrix4::Translation(-Eye) * Matrix4::ViewFromBasis(r, u, f);
}

inline Matrix4 LookAtRH(const Float3& Eye, const Float3& Target, const Float3& Up)
{
    const Float3 z = normalize(Eye - Target);
    const Float3 x = normalize(cross(Up, z));
    const Float3 y = cross(z, x);
    return Matrix4::Translation(-Eye) * Matrix4::ViewFromBasis(x, y, z);
}

inline void GetBasis(const Quaternion& rotation, Float3& right, Float3& up, Float3& forward)
{
    auto m = rotation.ToMatrix();
    right = Diligent::normalize<Float3>({m.m00, m.m01, m.m02});
    up = Diligent::normalize<Float3>({m.m10, m.m11, m.m12});
    forward = Diligent::normalize<Float3>({m.m20, m.m21, m.m22});
}
