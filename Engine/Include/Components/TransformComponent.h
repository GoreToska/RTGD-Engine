#pragma once
#include "Tools/Alias.h"

namespace RTGDEngine
{
    struct TransformComponent
    {
        TransformComponent(Float3 position = {0.0f, 0.0f, 0.0f}, Quaternion rotation = QuaternionIdentity(),
                           Float3 scale = {1.0f, 1.0f, 1.0f})
            : Position(position), Rotation(rotation), Scale(scale)
        {
        }

        Float3 Position = {0.0f, 0.0f, 0.0f};
        Quaternion Rotation = QuaternionIdentity();
        Float3 Scale = {1.0f, 1.0f, 1.0f};

        Matrix4 GetWorldMatrix() const
        {
            return Diligent::float4x4::Scale(Scale)
                   * Rotation.ToMatrix()
                   * Diligent::float4x4::Translation(Position);
        }

        void SetRotationEuler(const float pitch, const float yaw, const float roll)
        {
            Rotation = Quaternion::RotationFromAxisAngle({1, 0, 0}, pitch)
                       * Quaternion::RotationFromAxisAngle({0, 1, 0}, yaw)
                       * Quaternion::RotationFromAxisAngle({0, 0, 1}, roll);
        }

        void Rotate(const Float3 axis, const float angleRad)
        {
            Rotation = Rotation
                       * Quaternion::RotationFromAxisAngle(axis, angleRad);
            Rotation = Diligent::normalize(Rotation);
        }
    };
}
