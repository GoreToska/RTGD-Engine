#pragma once
#include "Tools/Alias.h"
#include "Engine/Reflection.h"

namespace RTGDEngine
{
    enum ECoordinateSpace
    {
        Local,
        World
    };


    struct TransformComponent
    {
        explicit TransformComponent(Float3 position = {0.0f, 0.0f, 0.0f}, Quaternion rotation = QuaternionIdentity(),
                           Float3 scale = {1.0f, 1.0f, 1.0f})
            : Position(position), Rotation(rotation), Scale(scale)
        {
        }

        Float3 Position = {0.0f, 0.0f, 0.0f};
        Quaternion Rotation = QuaternionIdentity();
        Float3 Scale = {1.0f, 1.0f, 1.0f};

        static constexpr Float3 GlobalForward = {0.0f, 0.0f, 1.0f};
        static constexpr Float3 GlobalUp = {0.0f, 1.0f, 0.0f};
        static constexpr Float3 GlobalRight = {1.0f, 0.0f, 0.0f};

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

        void Rotate(const Float3 axis, const float angleDeg, ECoordinateSpace coordinateSpace = Local)
        {
            if (coordinateSpace == Local)
                RotateLocal(axis, angleDeg);
            else if (coordinateSpace == World)
                RotateWorld(axis, angleDeg);
        }

        void RotateLocal(const Float3 axis, const float angleDeg)
        {
            Rotation = Rotation
                       * Quaternion::RotationFromAxisAngle(axis, angleDeg * Diligent::PI / 180.f);
            Rotation = Diligent::normalize(Rotation);
        }

        void RotateWorld(const Float3 axis, const float angleDeg)
        {
            Rotation = Quaternion::RotationFromAxisAngle(axis, angleDeg * Diligent::PI / 180.f)
                       * Rotation;
            Rotation = Diligent::normalize(Rotation);
        }

        Float3 GetForward() const
        {
            auto rotation = Rotation.ToMatrix();
            return Diligent::normalize<Float3>({rotation.m20, rotation.m21, rotation.m22});
        }

        Float3 GetRight() const
        {
            auto rotation = Rotation.ToMatrix();
            return Diligent::normalize<Float3>({rotation.m00, rotation.m01, rotation.m02});
        }

        Float3 GetUp() const
        {
            auto rotation = Rotation.ToMatrix();
            return Diligent::normalize<Float3>({rotation.m10, rotation.m11, rotation.m12});
        }
    };
}
