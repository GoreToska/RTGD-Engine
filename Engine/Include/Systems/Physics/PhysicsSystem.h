//
// Created by ivan on 8/20/26.
//
#pragma once
#include "Tools/RTGDMacros.h"
#include <flecs.h>

#include <Jolt/Jolt.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include "PhysicsLayer.h"


namespace RTGDEngine {
    class PhysicsSystem {
        DECLARE_SINGLETON(PhysicsSystem);

    public:
        void Initialize();

        void Update(flecs::world &world, float deltaTime);

        void Shutdown();

        JPH::BodyInterface &GetBodyInterface();

    private:
        BPLayerInterfaceImpl m_broadPhaseLayerInterface;
        ObjectVsBroadPhaseLayerFilterImpl m_objectVsBroadPhaseLayerFilter;
        ObjectLayerPairFilterImpl m_objectLayerPairFilter;
        std::unique_ptr<JPH::TempAllocatorImpl> m_tempAllocator;
        std::unique_ptr<JPH::JobSystemThreadPool> m_jobSystem;
        JPH::PhysicsSystem m_physicsSystem;

        int m_collisionSteps = 1;
    };

    DECLARE_GLOBAL_SINGLETON(PhysicsSystem, GPhysics);
} // RTGDEngine
