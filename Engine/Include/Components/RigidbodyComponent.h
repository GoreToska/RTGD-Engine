//
// Created by ivan on 8/20/26.
//

#pragma once

#include <flecs.h>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

#include "ColliderComponent.h"
#include "Systems/Physics/PhysicsSystem.h"
#include "Systems/Physics/PhysicsLayer.h"
#include "TransformComponent.h"
#include "Event/Events.h"
#include "Tools/Alias.h"
#include "Event/Delegate.h"
#include "Tools/JoltConversions.h"

namespace RTGDEngine
{
    enum class EMotionType { Static, Dynamic, Kinematic };

    enum class EPhysicsDOF : uint8_t
    {
        None = 0,
        TranslationX = 1 << 0,
        TranslationY = 1 << 1,
        TranslationZ = 1 << 2,
        RotationX = 1 << 3,
        RotationY = 1 << 4,
        RotationZ = 1 << 5,
        All = TranslationX | TranslationY | TranslationZ | RotationX | RotationY | RotationZ,
    };

    constexpr EPhysicsDOF operator|(EPhysicsDOF a, EPhysicsDOF b)
    {
        return static_cast<EPhysicsDOF>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    struct RigidbodyComponent
    {
        EMotionType MotionType = EMotionType::Dynamic;
        float Mass = 1.0f;
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

        void AddForce(const Float3& force) const
        {
            GPhysics.AddForce(BodyID, force);
        }

        void AddForceAtPosition(const Float3& force, const Float3& position) const
        {
            GPhysics.AddForceAtPosition(BodyID, force, position);
        }

        void AddTorque(const Float3& torque) const
        {
            GPhysics.AddTorque(BodyID, torque);
        }

        void AddImpulse(const Float3& impulse) const
        {
            GPhysics.AddImpulse(BodyID, impulse);
        }

        void AddImpulseAtPosition(const Float3& impulse, const Float3& position) const
        {
            GPhysics.AddImpulseAtPosition(BodyID, impulse, position);
        }

        void AddAngularImpulse(const Float3& impulse) const
        {
            GPhysics.AddAngularImpulse(BodyID, impulse);
        }

        static void RegisterMeta(const flecs::world& world)
        {
            world.component<EPhysicsDOF>()
                    .constant("TranslationX", EPhysicsDOF::TranslationX)
                    .constant("TranslationY", EPhysicsDOF::TranslationY)
                    .constant("TranslationZ", EPhysicsDOF::TranslationZ)
                    .constant("RotationX", EPhysicsDOF::RotationX)
                    .constant("RotationY", EPhysicsDOF::RotationY)
                    .constant("RotationZ", EPhysicsDOF::RotationZ)
                    .constant("All", EPhysicsDOF::All);

            world.component<EMotionType>()
                    .constant("Static", EMotionType::Static)
                    .constant("Dynamic", EMotionType::Dynamic)
                    .constant("Kinematic", EMotionType::Kinematic);

            flecs::component<RigidbodyComponent>(world, "RigidbodyComponent")
                    .member<EMotionType>("MotionType")
                    .member<float>("Mass")
                    .member<EPhysicsDOF>("Allowed DOFs");

            auto createBody = [](flecs::entity e)
            {
                auto collider = e.get_ref<ColliderComponent>();
                auto rb = e.get_ref<RigidbodyComponent>();
                auto xf = e.get_ref<TransformComponent>();
                if (!rb || !xf || !collider)
                    return;

                auto& bi = GPhysics.GetBodyInterface();
                if (!rb->BodyID.IsInvalid())
                {
                    GPhysics.UnregisterBody(rb->BodyID);
                    bi.RemoveBody(rb->BodyID);
                    bi.DestroyBody(rb->BodyID);
                }

                JPH::ShapeRefC shape = ColliderComponent::MakeShape(collider->Shape, collider->Extents);
                JPH::EMotionType motion = rb->MotionType == EMotionType::Static
                                              ? JPH::EMotionType::Static
                                              : rb->MotionType == EMotionType::Kinematic
                                                    ? JPH::EMotionType::Kinematic
                                                    : JPH::EMotionType::Dynamic;
                bool isMoving = rb->MotionType != EMotionType::Static;
                JPH::ObjectLayer layer = Layers::Encode(collider->Layer, isMoving);

                JPH::BodyCreationSettings settings(shape, ToRVec3(xf->Position), ToQuat(xf->Rotation), motion, layer);
                settings.mIsSensor = collider->IsTrigger;
                settings.mAllowedDOFs = static_cast<JPH::EAllowedDOFs>(rb->AllowedDOFs);

                if (rb->MotionType != EMotionType::Static)
                {
                    settings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
                    settings.mMassPropertiesOverride = ColliderComponent::ComputeMassProperties(
                        collider->Shape, collider->Extents, rb->Mass);
                }

                rb->BodyID = bi.CreateAndAddBody(settings,
                                                 rb->MotionType == EMotionType::Static
                                                     ? JPH::EActivation::DontActivate
                                                     : JPH::EActivation::Activate);
                GPhysics.RegisterBody(rb->BodyID, e.id(), collider->IsTrigger);
            };

            world.observer<RigidbodyComponent>().event(flecs::OnSet).each(
                [createBody](flecs::entity e, RigidbodyComponent&) { createBody(e); });

            world.observer<TransformComponent>().event(flecs::OnSet).each(
                [createBody](flecs::entity e, TransformComponent&)
                {
                    auto phys = e.get_ref<RigidbodyComponent>();
                    if (phys && phys->BodyID.IsInvalid())
                    {
                        createBody(e);
                    }
                });

            world.observer<RigidbodyComponent>().event(flecs::OnRemove).each(
                [](flecs::entity e, RigidbodyComponent& c)
                {
                    if (c.BodyID.IsInvalid())
                        return;
                    GPhysics.UnregisterBody(c.BodyID);
                    auto& bi = GPhysics.GetBodyInterface();
                    bi.RemoveBody(c.BodyID);
                    bi.DestroyBody(c.BodyID);
                });
        }
    };
}
