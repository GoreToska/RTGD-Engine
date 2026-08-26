//
// Created by ivan on 8/20/26.
//
#pragma once
#include "Tools/RTGDMacros.h"
#include <flecs.h>

#include <mutex>
#include <span>
#include <unordered_map>
#include <vector>
#include "Event/Events.h"

#include <Jolt/Jolt.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include "PhysicsLayer.h"
#include "Jolt/Physics/Collision/BroadPhase/BroadPhaseLayerInterfaceTable.h"
#include "Jolt/Physics/Collision/BroadPhase/ObjectVsBroadPhaseLayerFilterTable.h"


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

    struct RaycastHit {
        bool Hit = false;
        Entity Target;
        Float3 Point;
        Float3 Normal;
        float Distance = 0.0f;
        JPH::BodyID BodyID; // may be accessed through rigid body component from Target entity
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

        const std::vector<std::string> &GetLayerNames() const { return m_layers.GetNames(); }
        int GetLayerIndex(std::string_view layerName) const { return m_layers.GetIndex(layerName); };

        void RegisterBody(JPH::BodyID id, uint64_t entity, bool IsTrigger);

        void UnregisterBody(JPH::BodyID id);

        RaycastHit Raycast(World &world, const Float3 &origin, const Float3 &direction, float distance,
                           bool hitTriggers = false, uint32_t layerMask = ~0u, std::span<const JPH::BodyID> ignore = {});

        RaycastHit Raycast(World &world, const Float3 &origin, const Float3 &direction, float distance,
                           bool hitTriggers = false, uint32_t layerMask = ~0u, std::span<const Entity> ignore = {});

        RaycastHit Raycast(const Float3 &origin, const Float3 &direction, float distance,
                           bool hitTriggers = false, uint32_t layerMask = ~0u, std::span<const JPH::BodyID> ignore = {});

        RaycastHit Raycast(const Float3 &origin, const Float3 &direction, float distance,
                           bool hitTriggers = false, uint32_t layerMask = ~0u, std::span<const Entity> ignore = {});

        std::vector<RaycastHit> RaycastAll(World &world, const Float3 &origin, const Float3 &direction, float distance,
                                           bool hitTriggers = false, uint32_t layerMask = ~0u, std::span<const JPH::BodyID> ignore = {});

        std::vector<RaycastHit> RaycastAll(const Float3 &origin, const Float3 &direction, float distance,
                                           bool hitTriggers = false, uint32_t layerMask = ~0u, std::span<const JPH::BodyID> ignore = {});

        std::vector<RaycastHit> RaycastAll(World &world, const Float3 &origin, const Float3 &direction, float distance,
                                           bool hitTriggers = false, uint32_t layerMask = ~0u, std::span<const Entity> ignore = {});

        std::vector<RaycastHit> RaycastAll(const Float3 &origin, const Float3 &direction, float distance,
                                           bool hitTriggers = false, uint32_t layerMask = ~0u, std::span<const Entity> ignore = {});

        RaycastHit SphereCast(World &world, const Float3 &origin, const Float3 &direction, float radius, float distance,
                              bool hitTriggers = false, uint32_t layerMask = ~0u, std::span<const JPH::BodyID> ignore = {});

        RaycastHit SphereCast(World &world, const Float3 &origin, const Float3 &direction, float radius, float distance,
                              bool hitTriggers = false, uint32_t layerMask = ~0u, std::span<const Entity> ignore = {});

        RaycastHit SphereCast(const Float3 &origin, const Float3 &direction, float radius, float distance,
                              bool hitTriggers = false, uint32_t layerMask = ~0u, std::span<const JPH::BodyID> ignore = {});

        RaycastHit SphereCast(const Float3 &origin, const Float3 &direction, float radius, float distance,
                              bool hitTriggers = false, uint32_t layerMask = ~0u, std::span<const Entity> ignore = {});

        std::vector<RaycastHit> SphereCastAll(World &world, const Float3 &origin, const Float3 &direction, float radius,
                                              float distance,
                                              bool hitTriggers = false, uint32_t layerMask = ~0u, std::span<const JPH::BodyID> ignore = {});

        std::vector<RaycastHit> SphereCastAll(World &world, const Float3 &origin, const Float3 &direction, float radius,
                                              float distance,
                                              bool hitTriggers = false, uint32_t layerMask = ~0u, std::span<const Entity> ignore = {});

        std::vector<RaycastHit> SphereCastAll(const Float3 &origin, const Float3 &direction, float radius,
                                              float distance,
                                              bool hitTriggers = false, uint32_t layerMask = ~0u, std::span<const JPH::BodyID> ignore = {});

        std::vector<RaycastHit> SphereCastAll(const Float3 &origin, const Float3 &direction, float radius,
                                              float distance,
                                              bool hitTriggers = false, uint32_t layerMask = ~0u, std::span<const Entity> ignore = {});

        RaycastHit BoxCast(World &world, const Float3 &origin, const Float3 &direction, const Float3 &halfExtent,
                           float distance, const Quaternion &rotation = QuaternionIdentity(),
                           bool hitTriggers = false, uint32_t layerMask = ~0u, std::span<const JPH::BodyID> ignore = {});

        RaycastHit BoxCast(World &world, const Float3 &origin, const Float3 &direction, const Float3 &halfExtent,
                           float distance, const Quaternion &rotation = QuaternionIdentity(),
                           bool hitTriggers = false, uint32_t layerMask = ~0u, std::span<const Entity> ignore = {});

        RaycastHit BoxCast(const Float3 &origin, const Float3 &direction, const Float3 &halfExtent, float distance,
                           const Quaternion &rotation = QuaternionIdentity(),
                           bool hitTriggers = false, uint32_t layerMask = ~0u, std::span<const JPH::BodyID> ignore = {});

        RaycastHit BoxCast(const Float3 &origin, const Float3 &direction, const Float3 &halfExtent, float distance,
                           const Quaternion &rotation = QuaternionIdentity(),
                           bool hitTriggers = false, uint32_t layerMask = ~0u, std::span<const Entity> ignore = {});

        std::vector<RaycastHit> BoxCastAll(World &world, const Float3 &origin, const Float3 &direction,
                                           const Float3 &halfExtent,
                                           float distance, const Quaternion &rotation = QuaternionIdentity(),
                                           bool hitTriggers = false, uint32_t layerMask = ~0u, std::span<const JPH::BodyID> ignore = {});

        std::vector<RaycastHit> BoxCastAll(World &world, const Float3 &origin, const Float3 &direction,
                                           const Float3 &halfExtent,
                                           float distance, const Quaternion &rotation = QuaternionIdentity(),
                                           bool hitTriggers = false, uint32_t layerMask = ~0u, std::span<const Entity> ignore = {});

        std::vector<RaycastHit> BoxCastAll(const Float3 &origin, const Float3 &direction, const Float3 &halfExtent,
                                           float distance,
                                           const Quaternion &rotation = QuaternionIdentity(),
                                           bool hitTriggers = false, uint32_t layerMask = ~0u, std::span<const JPH::BodyID> ignore = {});

        std::vector<RaycastHit> BoxCastAll(const Float3 &origin, const Float3 &direction, const Float3 &halfExtent,
                                           float distance,
                                           const Quaternion &rotation = QuaternionIdentity(),
                                           bool hitTriggers = false, uint32_t layerMask = ~0u, std::span<const Entity> ignore = {});

    private:
        friend class ContactListenerImpl;

        struct BodyInfo {
            uint64_t Entity;
            bool IsTrigger;
        };

        RaycastHit MakeHit(World &world, JPH::BodyID id, float fraction, const JPH::RRayCast &ray,
                           JPH::SubShapeID subShape);

        RaycastHit MakeHit(World &world, JPH::ShapeCastResult &result, JPH::RVec3Arg baseOffset, float castLength);


        void QueueContact(JPH::BodyID id1, JPH::BodyID id2, JPH::RVec3 point, JPH::Vec3 normal, EContactPhase phase);

        void DispatchContact(World &world, const PendingContact &contact);

        void EmitStay(World &world, const PendingContact &contact);

        static uint64_t MakeContactKey(JPH::BodyID id1, JPH::BodyID id2);

        Layers::LayerRegistry m_layers;

        std::unique_ptr<JPH::TempAllocatorImpl> m_tempAllocator;
        std::unique_ptr<JPH::JobSystemThreadPool> m_jobSystem;
        JPH::PhysicsSystem m_physicsSystem;

        std::unordered_map<JPH::BodyID, BodyInfo> m_bodyInfo = {};
        std::mutex m_contactMutex = {};
        std::vector<PendingContact> m_pendingContacts = {};
        std::unordered_map<uint64_t, PendingContact> m_activeContacts = {};
        ContactListenerImpl m_contactListener = {};

        int m_collisionSteps = 1;
    };

    DECLARE_GLOBAL_SINGLETON(PhysicsSystem, GPhysics);
} // RTGDEngine
