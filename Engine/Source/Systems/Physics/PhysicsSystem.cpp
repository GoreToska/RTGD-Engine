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
    void ContactListenerImpl::OnContactAdded(const JPH::Body &b1, const JPH::Body &b2, const JPH::ContactManifold &m,
                                             JPH::ContactSettings &contact_settings) {
        Owner->QueueContact(b1.GetID(), b2.GetID(), m.GetWorldSpaceContactPointOn1(0), m.mWorldSpaceNormal,
                            EContactPhase::Enter);
    }

    void ContactListenerImpl::OnContactPersisted(const JPH::Body &b1, const JPH::Body &b2,
                                                 const JPH::ContactManifold &m,
                                                 JPH::ContactSettings &contact_settings) {
        Owner->QueueContact(b1.GetID(), b2.GetID(), m.GetWorldSpaceContactPointOn1(0), m.mWorldSpaceNormal,
                            EContactPhase::Stay);
    }

    void ContactListenerImpl::OnContactRemoved(const JPH::SubShapeIDPair &pair) {
        Owner->QueueContact(pair.GetBody1ID(), pair.GetBody2ID(), JPH::RVec3::sZero(), JPH::Vec3::sZero(),
                            EContactPhase::Exit);
    }

    void PhysicsSystem::RegisterBody(JPH::BodyID id, uint64_t entity, bool IsTrigger) {
        m_bodyInfo[id] = {entity, IsTrigger};
    }

    void PhysicsSystem::UnregisterBody(JPH::BodyID id) {
        m_bodyInfo.erase(id);
        std::erase_if(m_activeContacts, [id](const auto &kv) {
            return kv.second.Body1 == id || kv.second.Body2 == id;
        });
    }

    void PhysicsSystem::QueueContact(JPH::BodyID id1, JPH::BodyID id2, JPH::RVec3 point, JPH::Vec3 normal,
                                     EContactPhase phase) {
        std::lock_guard lock(m_contactMutex);
        m_pendingContacts.push_back({id1, id2, point, normal, phase});
    }

    uint64_t PhysicsSystem::MakeContactKey(JPH::BodyID id1, JPH::BodyID id2) {
        uint32_t a = id1.GetIndexAndSequenceNumber(), b = id2.GetIndexAndSequenceNumber();
        if (a > b) std::swap(a, b);
        return (uint64_t(a) << 32) | b;
    }

    void PhysicsSystem::EmitStay(::World &world, const PendingContact &contact) {
        auto it1 = m_bodyInfo.find(contact.Body1);
        auto it2 = m_bodyInfo.find(contact.Body2);
        if (it1 == m_bodyInfo.end() || it2 == m_bodyInfo.end()) return;

        bool isTrigger = it1->second.IsTrigger || it2->second.IsTrigger;
        Entity e1 = world.entity(it1->second.Entity);
        Entity e2 = world.entity(it2->second.Entity);
        Float3 point = ToFloat3(contact.Point), normal = ToFloat3(contact.Normal);

        if (isTrigger) {
            Events::TriggerStayEvent evt1{e1, e2, point, normal}, evt2{e2, e1, point, normal};
            GEventBus.Emit(Events::OnTriggerStay, evt1, EmitBadge<PhysicsSystem>{});
            if (auto p1 = e1.get_ref<PhysicsComponent>()) p1->OnTriggerStay.Broadcast({}, evt1);
            if (auto p2 = e2.get_ref<PhysicsComponent>()) p2->OnTriggerStay.Broadcast({}, evt2);
        } else {
            Events::CollisionStayEvent evt1{e1, e2, point, normal}, evt2{e2, e1, point, normal};
            GEventBus.Emit(Events::OnCollisionStay, evt1, EmitBadge<PhysicsSystem>{});
            if (auto p1 = e1.get_ref<PhysicsComponent>()) p1->OnCollisionStay.Broadcast({}, evt1);
            if (auto p2 = e2.get_ref<PhysicsComponent>()) p2->OnCollisionStay.Broadcast({}, evt2);
        }
    }

    void PhysicsSystem::DispatchContact(::World &world, const PendingContact &contact) {
        auto it1 = m_bodyInfo.find(contact.Body1);
        auto it2 = m_bodyInfo.find(contact.Body2);

        if (it1 == m_bodyInfo.end() || it2 == m_bodyInfo.end()) return;

        bool isTrigger = it1->second.IsTrigger || it2->second.IsTrigger;
        Entity e1 = world.entity(it1->second.Entity);
        Entity e2 = world.entity(it2->second.Entity);

        switch (contact.Phase) {
            case EContactPhase::Enter: {
                Float3 point = ToFloat3(contact.Point), normal = ToFloat3(contact.Normal);
                if (isTrigger) {
                    Events::TriggerEnterEvent evt1{e1, e2, point, normal}, evt2{e2, e1, point, normal};
                    GEventBus.Emit(Events::OnTriggerEnter, evt1, EmitBadge<PhysicsSystem>{});
                    if (auto p1 = e1.get_ref<PhysicsComponent>()) p1->OnTriggerEnter.Broadcast({}, evt1);
                    if (auto p2 = e2.get_ref<PhysicsComponent>()) p2->OnTriggerEnter.Broadcast({}, evt2);
                } else {
                    Events::CollisionEnterEvent evt1{e1, e2, point, normal}, evt2{e2, e1, point, normal};
                    GEventBus.Emit(Events::OnCollisionEnter, evt1, EmitBadge<PhysicsSystem>{});
                    if (auto p1 = e1.get_ref<PhysicsComponent>()) p1->OnCollisionEnter.Broadcast({}, evt1);
                    if (auto p2 = e2.get_ref<PhysicsComponent>()) p2->OnCollisionEnter.Broadcast({}, evt2);
                }
                m_activeContacts[MakeContactKey(contact.Body1, contact.Body2)] = contact;
                break;
            }
            case EContactPhase::Stay: {
                // Jolt stops calling OnContactPersisted once a touching pair sleeps; the
                // Stay event itself is re-emitted every tick from m_activeContacts in
                // Update(), so this just refreshes the cached point/normal while awake.
                m_activeContacts[MakeContactKey(contact.Body1, contact.Body2)] = contact;
                break;
            }
            case EContactPhase::Exit: {
                auto &bi = GetBodyInterface();
                if (!bi.IsActive(contact.Body1) && !bi.IsActive(contact.Body2)) break;

                m_activeContacts.erase(MakeContactKey(contact.Body1, contact.Body2));

                if (isTrigger) {
                    Events::TriggerExitEvent evt1{e1, e2}, evt2{e2, e1};
                    GEventBus.Emit(Events::OnTriggerExit, evt1, EmitBadge<PhysicsSystem>{});
                    if (auto p1 = e1.get_ref<PhysicsComponent>()) p1->OnTriggerExit.Broadcast({}, evt1);
                    if (auto p2 = e2.get_ref<PhysicsComponent>()) p2->OnTriggerExit.Broadcast({}, evt2);
                } else {
                    Events::CollisionExitEvent evt1{e1, e2}, evt2{e2, e1};
                    GEventBus.Emit(Events::OnCollisionExit, evt1, EmitBadge<PhysicsSystem>{});
                    if (auto p1 = e1.get_ref<PhysicsComponent>()) p1->OnCollisionExit.Broadcast({}, evt1);
                    if (auto p2 = e2.get_ref<PhysicsComponent>()) p2->OnCollisionExit.Broadcast({}, evt2);
                }
                break;
            }
        }
    }

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

        m_contactListener.Owner = this;
        m_physicsSystem.SetContactListener(&m_contactListener);
    }

    void PhysicsSystem::Update(flecs::world &world, float deltaTime) {
        world.query<PhysicsComponent, TransformComponent>().each(
            [&](PhysicsComponent &physics, TransformComponent &transform) {
                if (physics.MotionType == EMotionType::Static || physics.BodyID.IsInvalid()) {
                    return;
                }

                auto &bi = m_physicsSystem.GetBodyInterface();
                bi.SetLinearVelocity(physics.BodyID, ToVec3(physics.Velocity));
                bi.SetAngularVelocity(physics.BodyID, ToVec3(physics.AngularVelocity));
            });

        m_physicsSystem.Update(deltaTime, m_collisionSteps, m_tempAllocator.get(), m_jobSystem.get());

        std::vector<PendingContact> contacts = {};
        {
            std::lock_guard<std::mutex> lock(m_contactMutex);
            contacts.swap(m_pendingContacts);
        }

        for (auto &c: contacts) {
            DispatchContact(world, c);
        }

        for (auto &[key, c]: m_activeContacts) {
            EmitStay(world, c);
        }

        world.query<PhysicsComponent, TransformComponent>().each(
            [&](PhysicsComponent &physics, TransformComponent &transform) {
                if (physics.MotionType == EMotionType::Static || physics.BodyID.IsInvalid()) {
                    return;
                }

                auto &bi = m_physicsSystem.GetBodyInterface();
                transform.Position = ToFloat3(bi.GetPosition(physics.BodyID));
                transform.Rotation = ToQuaternion(bi.GetRotation(physics.BodyID));
                physics.Velocity = ToFloat3(bi.GetLinearVelocity(physics.BodyID));
                physics.AngularVelocity = ToFloat3(bi.GetAngularVelocity(physics.BodyID));
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
