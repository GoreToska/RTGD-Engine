//
// Created by ivan on 8/25/26.
//

#pragma once
#include <Jolt/Jolt.h>

#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/Physics/Collision/Shape/Shape.h"
#include "Jolt/Physics/Collision/Shape/SphereShape.h"
#include "Tools/Alias.h"
#include "Tools/JoltConversions.h"

namespace RTGDEngine {
    enum class EPhysicsShape {
        Box,
        Sphere
    };

    struct ColliderComponent {
        EPhysicsShape Shape = EPhysicsShape::Box;
        Float3 Extents = {0.5f, 0.5f, 0.5f};
        bool IsTrigger = false;

        static JPH::ShapeRefC MakeShape(EPhysicsShape shape, const Float3 &extents) {
            switch (shape) {
                case EPhysicsShape::Box:
                    return JPH::BoxShapeSettings(ToVec3(extents)).Create().Get();
                case EPhysicsShape::Sphere:
                    return JPH::SphereShapeSettings(extents.x).Create().Get();
            }

            return nullptr;
        }

        static JPH::MassProperties ComputeMassProperties(EPhysicsShape shape, const Float3 &extents, float mass) {
            JPH::MassProperties props;
            switch (shape) {
                case EPhysicsShape::Box:
                    props.SetMassAndInertiaOfSolidBox(2.0f * ToVec3(extents), 1.0f);
                    break;
                case EPhysicsShape::Sphere: {
                    float radius = extents.x;
                    float inertia = 0.4f * radius * radius;
                    props.mMass = 1.0f;
                    props.mInertia = JPH::Mat44::sScale(JPH::Vec3(inertia, inertia, inertia));
                    break;
                }
            }

            props.ScaleToMass(mass);
            return props;
        }

        static void RegisterMeta(const World &world) {
            world.component<EPhysicsShape>("PhysicsShape")
                    .constant("Box", EPhysicsShape::Box)
                    .constant("Sphere", EPhysicsShape::Sphere);

            world.component<ColliderComponent>("ColliderComponent")
                    .member<EPhysicsShape>("Shape")
                    .member<Float3>("Extents")
                    .member<bool>("IsTrigger");
        }
    };
}
