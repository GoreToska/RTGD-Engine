//
// Created by ivan on 8/20/26.
//
#pragma once
#include "Tools/RTGDMacros.h"
#include <flecs.h>

#include <mutex>
#include <unordered_map>
#include <vector>
#include "Event/Events.h"

#include <Jolt/Jolt.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include "PhysicsLayer.h"


namespace RTGDEngine {
    enum class EContactPhase {
        Enter,
        Stay,
        Exit
    };

    struct PendingContact {
        JPH::BodyID Body1, Body2;
        JPH::RVec3 Point;
        JPH::Vec3 Normal;
        EContactPhase Phase;
    };

    class ContactListenerImpl final : public JPH::ContactListener {
    public:
        PhysicsSystem *Owner = nullptr;

        void OnContactAdded(const JPH::Body &b1, const JPH::Body &b2, const JPH::ContactManifold &m,
                            JPH::ContactSettings &) override;

        void OnContactPersisted(const JPH::Body &b1, const JPH::Body &b2, const JPH::ContactManifold &m,
                                JPH::ContactSettings &) override;

        void OnContactRemoved(const JPH::SubShapeIDPair &pair) override;
    };

    class PhysicsSystem {
        DECLARE_SINGLETON(PhysicsSystem);

    public:
        void Initialize();

        void Update(flecs::world &world, float deltaTime);

        void Shutdown();

        JPH::BodyInterface &GetBodyInterface();

        void RegisterBody(JPH::BodyID id, uint64_t entity, bool IsTrigger);

        void UnregisterBody(JPH::BodyID id);

    private:
        friend class ContactListenerImpl;

        struct BodyInfo {
            uint64_t Entity;
            bool IsTrigger;
        };

        void QueueContact(JPH::BodyID id1, JPH::BodyID id2, JPH::RVec3 point, JPH::Vec3 normal, EContactPhase phase);

        void DispatchContact(World &world, const PendingContact &contact);

        BPLayerInterfaceImpl m_broadPhaseLayerInterface;
        ObjectVsBroadPhaseLayerFilterImpl m_objectVsBroadPhaseLayerFilter;
        ObjectLayerPairFilterImpl m_objectLayerPairFilter;
        std::unique_ptr<JPH::TempAllocatorImpl> m_tempAllocator;
        std::unique_ptr<JPH::JobSystemThreadPool> m_jobSystem;
        JPH::PhysicsSystem m_physicsSystem;

        std::unordered_map<JPH::BodyID, BodyInfo> m_bodyInfo = {};
        std::mutex m_contactMutex = {};
        std::vector<PendingContact> m_pendingContacts = {};
        ContactListenerImpl m_contactListener = {};

        int m_collisionSteps = 1;
    };

    DECLARE_GLOBAL_SINGLETON(PhysicsSystem, GPhysics);
} // RTGDEngine
