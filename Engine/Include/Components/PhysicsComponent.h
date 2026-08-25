//
// Created by ivan on 8/20/26.
//

#pragma once

#include <flecs.h>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

#include "Systems/Physics/PhysicsSystem.h"
#include "Systems/Physics/PhysicsLayer.h"
#include "TransformComponent.h"
#include "Event/Events.h"
#include "Tools/Alias.h"
#include "../Event/Delegate.h"
#include "Tools/JoltConversions.h"

namespace RTGDEngine {
    enum class EPhysicsShape {
        Box,
        Sphere
    };

    enum class EMotionType { Static, Dynamic, Kinematic };

    enum class EPhysicsDOF : uint8_t {
        None = 0,
        TranslationX = 1 << 0,
        TranslationY = 1 << 1,
        TranslationZ = 1 << 2,
        RotationX = 1 << 3,
        RotationY = 1 << 4,
        RotationZ = 1 << 5,
        All = TranslationX | TranslationY | TranslationZ | RotationX | RotationY | RotationZ,
    };

    constexpr EPhysicsDOF operator|(EPhysicsDOF a, EPhysicsDOF b) {
        return static_cast<EPhysicsDOF>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    struct PhysicsComponent {
        EPhysicsShape Shape = EPhysicsShape::Box;
        Float3 Extents = {0.5f, 0.5f, 0.5f}; // box - extents, sphere - radius (x)
        EMotionType MotionType = EMotionType::Dynamic;
        float Mass = 1.0f;
        bool IsTrigger = false;
        EPhysicsDOF AllowedDOFs = EPhysicsDOF::All;

        // transient
        JPH::BodyID BodyID;
        Float3 Velocity = {0, 0, 0};
        Float3 AngularVelocity = {0, 0, 0};
        Delegate<PhysicsSystem, const Events::CollisionEnterEvent &> OnCollisionEnter;
        Delegate<PhysicsSystem, const Events::CollisionStayEvent &> OnCollisionStay;
        Delegate<PhysicsSystem, const Events::CollisionExitEvent &> OnCollisionExit;
        Delegate<PhysicsSystem, const Events::TriggerEnterEvent &> OnTriggerEnter;
        Delegate<PhysicsSystem, const Events::TriggerStayEvent &> OnTriggerStay;
        Delegate<PhysicsSystem, const Events::TriggerExitEvent &> OnTriggerExit;

        void AddVelocity(const Float3 &velocity) {
            Velocity += velocity;
        }

        static void RegisterMeta(const flecs::world &world) {
            world.component<EPhysicsDOF>()
                    .constant("TranslationX", EPhysicsDOF::TranslationX)
                    .constant("TranslationY", EPhysicsDOF::TranslationY)
                    .constant("TranslationZ", EPhysicsDOF::TranslationZ)
                    .constant("RotationX", EPhysicsDOF::RotationX)
                    .constant("RotationY", EPhysicsDOF::RotationY)
                    .constant("RotationZ", EPhysicsDOF::RotationZ)
                    .constant("All", EPhysicsDOF::All);

            world.component<EPhysicsShape>()
                    .constant("Box", EPhysicsShape::Box)
                    .constant("Sphere", EPhysicsShape::Sphere);

            world.component<EMotionType>()
                    .constant("Static", EMotionType::Static)
                    .constant("Dynamic", EMotionType::Dynamic)
                    .constant("Kinematic", EMotionType::Kinematic);

            flecs::component<PhysicsComponent>(world, "PhysicsComponent")
                    .member<EPhysicsShape>("Shape")
                    .member<Float3>("Extents")
                    .member<EMotionType>("MotionType")
                    .member<float>("Mass")
                    .member<bool>("IsTrigger")
                    .member<EPhysicsDOF>("Allowed DOFs");

            auto createBody = [](flecs::entity e) {
                auto phys = e.get_ref<PhysicsComponent>();
                auto xf = e.get_ref<TransformComponent>();
                if (!phys || !xf) return;

                auto &bi = GPhysics.GetBodyInterface();
                if (!phys->BodyID.IsInvalid()) {
                    GPhysics.UnregisterBody(phys->BodyID);
                    bi.RemoveBody(phys->BodyID);
                    bi.DestroyBody(phys->BodyID);
                }

                JPH::ShapeRefC shape;
                JPH::MassProperties massProps;

                switch (phys->Shape) {
                    case EPhysicsShape::Box: {
                        shape = JPH::BoxShapeSettings(ToVec3(phys->Extents)).Create().Get();
                        massProps.SetMassAndInertiaOfSolidBox(2.0f * ToVec3(phys->Extents), 1.0f);
                        break;
                    }
                    case EPhysicsShape::Sphere: {
                        float radius = phys->Extents.x;
                        shape = JPH::SphereShapeSettings(radius).Create().Get();
                        float inertia = 0.4f * radius * radius; // solid sphere: (2/5) m r^2
                        massProps.mMass = 1.0f;
                        massProps.mInertia = JPH::Mat44::sScale(JPH::Vec3(inertia, inertia, inertia));
                        break;
                    }
                }
                massProps.ScaleToMass(phys->Mass);

                JPH::EMotionType motion = phys->MotionType == EMotionType::Static
                                              ? JPH::EMotionType::Static
                                              : phys->MotionType == EMotionType::Kinematic
                                                    ? JPH::EMotionType::Kinematic
                                                    : JPH::EMotionType::Dynamic;
                JPH::ObjectLayer layer = phys->MotionType == EMotionType::Static ? Layers::NON_MOVING : Layers::MOVING;

                JPH::BodyCreationSettings settings(shape, ToRVec3(xf->Position), ToQuat(xf->Rotation), motion, layer);
                settings.mIsSensor = phys->IsTrigger;
                settings.mAllowedDOFs = static_cast<JPH::EAllowedDOFs>(phys->AllowedDOFs);

                if (phys->MotionType != EMotionType::Static) {
                    settings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
                    settings.mMassPropertiesOverride = massProps;
                }

                phys->BodyID = bi.CreateAndAddBody(settings,
                                                   phys->MotionType == EMotionType::Static
                                                       ? JPH::EActivation::DontActivate
                                                       : JPH::EActivation::Activate);
                GPhysics.RegisterBody(phys->BodyID, e.id(), phys->IsTrigger);
            };

            world.observer<PhysicsComponent>().event(flecs::OnSet).each(
                [createBody](flecs::entity e, PhysicsComponent &) { createBody(e); });

            world.observer<TransformComponent>().event(flecs::OnSet).each(
                [createBody](flecs::entity e, TransformComponent &) {
                    auto phys = e.get_ref<PhysicsComponent>();
                    if (phys && phys->BodyID.IsInvalid()) {
                        createBody(e);
                    }
                });

            world.observer<PhysicsComponent>().event(flecs::OnRemove).each(
                [](flecs::entity e, PhysicsComponent &c) {
                    if (c.BodyID.IsInvalid()) return;
                    GPhysics.UnregisterBody(c.BodyID);
                    auto &bi = GPhysics.GetBodyInterface();
                    bi.RemoveBody(c.BodyID);
                    bi.DestroyBody(c.BodyID);
                });
        }
    };
}
