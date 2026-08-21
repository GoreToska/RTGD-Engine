//
// Created by ivan on 8/20/26.
//

#include "Systems/Physics/PhysicsSystem.h"

#include <flecs.h>

#include "Components/PhysicsComponent.h"
#include "Components/TransformComponent.h"
#include "Jolt/RegisterTypes.h"
#include "Jolt/Core/Factory.h"
#include "Tools/JoltConversions.h"

namespace RTGDEngine {
    void PhysicsSystem::Initialize() {
        JPH::RegisterDefaultAllocator();
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();

        m_tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
        m_jobSystem = std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers,
                                                                 (int) std::thread::hardware_concurrency() - 1);

        const JPH::uint cMaxBodies = 1024;
        const JPH::uint cNumBodyMutexes = 0;
        const JPH::uint cMaxBodyPairs = 1024;
        const JPH::uint cMaxContactConstraints = 1024;

        m_physicsSystem.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints,
                             m_broadPhaseLayerInterface, m_objectVsBroadPhaseLayerFilter, m_objectLayerPairFilter);
        m_physicsSystem.SetGravity(JPH::Vec3(0.0f, -9.81f, 0.0f));
    }

    void PhysicsSystem::Update(flecs::world &world, float deltaTime) {
        m_physicsSystem.Update(deltaTime, m_collisionSteps, m_tempAllocator.get(), m_jobSystem.get());

        world.query<PhysicsComponent, TransformComponent>().each(
            [&](PhysicsComponent &physics, TransformComponent &transform) {
                if (physics.MotionType == EMotionType::Static || physics.BodyID.IsInvalid()) {
                    return;
                }

                auto &bi = m_physicsSystem.GetBodyInterface();
                transform.Position = ToFloat3(bi.GetPosition(physics.BodyID));
                transform.Rotation = ToQuaternion(bi.GetRotation(physics.BodyID));
            });
    }

    void PhysicsSystem::Shutdown() {
        JPH::UnregisterTypes();
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;
    }

    JPH::BodyInterface &PhysicsSystem::GetBodyInterface() {
        return m_physicsSystem.GetBodyInterface();
    }
} // RTGDEngine
