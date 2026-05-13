#pragma once
#include "Tools/Alias.h"
#include "Tools/MetaTypes.h"

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
            : Position(position),
              Rotation(rotation == Quaternion{0, 0, 0, 0} ? QuaternionIdentity() : rotation),
              Scale(scale)
        {
        }

        Float3 Position = {0.0f, 0.0f, 0.0f};
        Quaternion Rotation = QuaternionIdentity();
        Float3 Scale = {1.0f, 1.0f, 1.0f};

        static constexpr Float3 GlobalForward = {0.0f, 0.0f, 1.0f};
        static constexpr Float3 GlobalUp = {0.0f, 1.0f, 0.0f};
        static constexpr Float3 GlobalRight = {1.0f, 0.0f, 0.0f};

        Matrix4 GetWorldMatrix()
        {
            Rotation = Diligent::normalize(Rotation);

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

        void LookDirection(const Float3& direction, const Float3& worldUp = GlobalUp)
        {
            if (Diligent::length(direction) < 0.000001f)
                return;

            Float3 forward = Diligent::normalize(direction);
            Float3 right = Diligent::cross(worldUp, forward);

            if (Diligent::length(right) < 0.000001f)
            {
                right = Diligent::cross(GlobalRight, forward);

                if (Diligent::length(right) < 0.000001f)
                    right = Diligent::cross(GlobalForward, forward);
            }

            right = Diligent::normalize(right);

            Float3 up = Diligent::normalize(
                Diligent::cross(forward, right));

            const float m00 = right.x;
            const float m01 = right.y;
            const float m02 = right.z;

            const float m10 = up.x;
            const float m11 = up.y;
            const float m12 = up.z;

            const float m20 = forward.x;
            const float m21 = forward.y;
            const float m22 = forward.z;

            const float trace = m00 + m11 + m22;

            Quaternion q;

            if (trace > 0.0f)
            {
                float s = std::sqrt(trace + 1.0f) * 2.0f;

                q.q.w = 0.25f * s;
                q.q.x = (m12 - m21) / s;
                q.q.y = (m20 - m02) / s;
                q.q.z = (m01 - m10) / s;
            }
            else if (m00 > m11 && m00 > m22)
            {
                float s = std::sqrt(1.0f + m00 - m11 - m22) * 2.0f;

                q.q.w = (m12 - m21) / s;
                q.q.x = 0.25f * s;
                q.q.y = (m10 + m01) / s;
                q.q.z = (m20 + m02) / s;
            }
            else if (m11 > m22)
            {
                float s = std::sqrt(1.0f + m11 - m00 - m22) * 2.0f;

                q.q.w = (m20 - m02) / s;
                q.q.x = (m10 + m01) / s;
                q.q.y = 0.25f * s;
                q.q.z = (m21 + m12) / s;
            }
            else
            {
                float s = std::sqrt(1.0f + m22 - m00 - m11) * 2.0f;

                q.q.w = (m01 - m10) / s;
                q.q.x = (m20 + m02) / s;
                q.q.y = (m21 + m12) / s;
                q.q.z = 0.25f * s;
            }

            Rotation = Diligent::normalize(q);
        }

        void LookAt(const Float3& target, const Float3& worldUp = GlobalUp)
        {
            LookDirection(target - Position, worldUp);
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

        static void RegisterMeta(const flecs::world& world)
        {
            if (!MetaAlreadyRegistered(world, flecs::component<TransformComponent>(world, "TransformComponent")))
                flecs::component<TransformComponent>(world, "TransformComponent")
                        .member<Float3>("Position")
                        .member<Quaternion>("Rotation")
                        .member<Float3>("Scale");
        }
    };
}
