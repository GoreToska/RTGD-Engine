//
// Created by ivan on 8/20/26.
//

#include "Systems/Physics/PhysicsSystem.h"

#include <flecs.h>
#include <fstream>

#include "Jolt/Physics/Collision/CastResult.h"
#include "Jolt/Physics/Collision/CollisionCollectorImpl.h"
#include "Jolt/Physics/Collision/RayCast.h"
#include "Jolt/Physics/Collision/ShapeCast.h"
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/CollideShape.h>

#include "AssetLoader/PathResolve.h"
#include "Components/CharacterControllerComponent.h"
#include "Components/RigidbodyComponent.h"
#include "Components/TransformComponent.h"
#include "Jolt/RegisterTypes.h"
#include "Jolt/Core/Factory.h"
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"

#include "Scene/SceneManager.h"
#include "Tools/JoltConversions.h"
#include "Tools/Logger.h"

namespace
{
    class RaycastBodyFilter final : public JPH::BodyFilter
    {
    public:
        JPH::IgnoreMultipleBodiesFilter Ignore;
        bool HitTriggers = false;

        bool ShouldCollide(const JPH::BodyID& inBodyID) const override
        {
            return Ignore.ShouldCollide(inBodyID);
        }

        bool ShouldCollideLocked(const JPH::Body& inBody) const override
        {
            return HitTriggers || !inBody.IsSensor();
        }
    };

    class GameplayLayerMaskFilter final : public JPH::ObjectLayerFilter
    {
    public:
        uint32_t Mask = ~0u;

        bool ShouldCollide(JPH::ObjectLayer inLayer) const override
        {
            return (Mask & (1u << RTGDEngine::Layers::DecodeGameplayLayer(inLayer))) != 0;
        }
    };

    std::vector<JPH::BodyID> ResolveIgnoreList(std::span<const Entity> entities)
    {
        std::vector<JPH::BodyID> ids;
        ids.reserve(entities.size());
        for (auto& e: entities)
        {
            if (auto rb = e.get_ref<RTGDEngine::RigidbodyComponent>())
                ids.push_back(rb->BodyID);
            else if (auto cc = e.get_ref<RTGDEngine::CharacterControllerComponent>())
            {
                if (cc->Controller)
                {
                    JPH::BodyID id = cc->Controller->GetBodyID();
                    if (!id.IsInvalid())
                        ids.push_back(id);
                }
            }
        }

        return ids;
    }
}

namespace RTGDEngine
{
    void ContactListenerImpl::OnContactAdded(const JPH::Body& b1, const JPH::Body& b2, const JPH::ContactManifold& m,
                                             JPH::ContactSettings& contact_settings)
    {
        Owner->QueueContact(b1.GetID(), b2.GetID(), m.GetWorldSpaceContactPointOn1(0), m.mWorldSpaceNormal,
                            EContactPhase::Enter);
    }

    void ContactListenerImpl::OnContactPersisted(const JPH::Body& b1, const JPH::Body& b2,
                                                 const JPH::ContactManifold& m,
                                                 JPH::ContactSettings& contact_settings)
    {
        Owner->QueueContact(b1.GetID(), b2.GetID(), m.GetWorldSpaceContactPointOn1(0), m.mWorldSpaceNormal,
                            EContactPhase::Stay);
    }

    void ContactListenerImpl::OnContactRemoved(const JPH::SubShapeIDPair& pair)
    {
        Owner->QueueContact(pair.GetBody1ID(), pair.GetBody2ID(), JPH::RVec3::sZero(), JPH::Vec3::sZero(),
                            EContactPhase::Exit);
    }

    void PhysicsSystem::RegisterBody(JPH::BodyID id, uint64_t entity, bool IsTrigger)
    {
        m_bodyInfo[id] = {entity, IsTrigger};
    }

    void PhysicsSystem::UnregisterBody(JPH::BodyID id)
    {
        m_bodyInfo.erase(id);
        std::erase_if(m_activeContacts, [id](const auto& kv)
        {
            return kv.second.Body1 == id || kv.second.Body2 == id;
        });
    }

    void PhysicsSystem::AddForce(JPH::BodyID id, const Float3& force)
    {
        if (id.IsInvalid())
            return;
        m_pendingForceCommands.push_back({id, ToVec3(force), {}, EForceCommand::Force});
    }

    void PhysicsSystem::AddForceAtPosition(JPH::BodyID id, const Float3& force, const Float3& position)
    {
        if (id.IsInvalid())
            return;
        m_pendingForceCommands.push_back({id, ToVec3(force), ToRVec3(position), EForceCommand::ForceAtPosition});
    }

    void PhysicsSystem::AddTorque(JPH::BodyID id, const Float3& torque)
    {
        if (id.IsInvalid())
            return;
        m_pendingForceCommands.push_back({id, ToVec3(torque), {}, EForceCommand::Torque});
    }

    void PhysicsSystem::AddImpulse(JPH::BodyID id, const Float3& impulse)
    {
        if (id.IsInvalid())
            return;
        m_pendingForceCommands.push_back({id, ToVec3(impulse), {}, EForceCommand::Impulse});
    }

    void PhysicsSystem::AddImpulseAtPosition(JPH::BodyID id, const Float3& impulse, const Float3& position)
    {
        if (id.IsInvalid())
            return;
        m_pendingForceCommands.push_back({id, ToVec3(impulse), ToRVec3(position), EForceCommand::ImpulseAtPosition});
    }

    void PhysicsSystem::AddAngularImpulse(JPH::BodyID id, const Float3& impulse)
    {
        if (id.IsInvalid())
            return;
        m_pendingForceCommands.push_back({id, ToVec3(impulse), {}, EForceCommand::AngularImpulse});
    }

    RaycastHit PhysicsSystem::Raycast(World& world, const Float3& origin, const Float3& direction, float distance,
                                      bool hitTriggers, uint32_t layerMask, std::span<const JPH::BodyID> ignore)
    {
        JPH::RRayCast ray(ToRVec3(origin), ToVec3(direction).Normalized() * distance);

        RaycastBodyFilter filter;
        filter.HitTriggers = hitTriggers;
        for (auto id: ignore)
        {
            filter.Ignore.IgnoreBody(id);
        }

        GameplayLayerMaskFilter layerFilter;
        layerFilter.Mask = layerMask;

        JPH::RayCastResult result;
        if (!m_physicsSystem.GetNarrowPhaseQuery().CastRay(ray, result, {}, layerFilter, filter))
            return {};

        return MakeHit(world, result.mBodyID, result.mFraction, ray, result.mSubShapeID2);
    }

    RaycastHit PhysicsSystem::Raycast(World& world, const Float3& origin, const Float3& direction, float distance,
                                      bool hitTriggers, uint32_t layerMask, std::span<const Entity> ignore)
    {
        std::vector<JPH::BodyID> ids = ResolveIgnoreList(ignore);
        return Raycast(world, origin, direction, distance, hitTriggers, layerMask, ids);
    }

    RaycastHit PhysicsSystem::Raycast(const Float3& origin, const Float3& direction, float distance, bool hitTriggers,
                                      uint32_t layerMask, std::span<const JPH::BodyID> ignore)
    {
        return Raycast(GScene.GetWorld(), origin, direction, distance, hitTriggers, layerMask, ignore);
    }

    RaycastHit PhysicsSystem::Raycast(const Float3& origin, const Float3& direction, float distance, bool hitTriggers,
                                      uint32_t layerMask, std::span<const Entity> ignore)
    {
        return Raycast(GScene.GetWorld(), origin, direction, distance, hitTriggers, layerMask, ignore);
    }

    std::vector<RaycastHit> PhysicsSystem::RaycastAll(World& world, const Float3& origin, const Float3& direction,
                                                      float distance, bool hitTriggers, uint32_t layerMask,
                                                      std::span<const JPH::BodyID> ignore)
    {
        JPH::RRayCast ray(ToRVec3(origin), ToVec3(direction).Normalized() * distance);

        RaycastBodyFilter filter;
        filter.HitTriggers = hitTriggers;
        for (auto id: ignore)
            filter.Ignore.IgnoreBody(id);

        GameplayLayerMaskFilter layerFilter;
        layerFilter.Mask = layerMask;

        JPH::AllHitCollisionCollector<JPH::CastRayCollector> collector;
        m_physicsSystem.GetNarrowPhaseQuery().CastRay(ray, {}, collector, {}, layerFilter, filter);

        std::vector<RaycastHit> hits;
        hits.reserve(collector.mHits.size());
        for (auto& h: collector.mHits)
            hits.push_back(MakeHit(world, h.mBodyID, h.mFraction, ray, h.mSubShapeID2));

        return hits;
    }

    std::vector<RaycastHit> PhysicsSystem::RaycastAll(const Float3& origin, const Float3& direction, float distance,
                                                      bool hitTriggers, uint32_t layerMask,
                                                      std::span<const JPH::BodyID> ignore)
    {
        return RaycastAll(GScene.GetWorld(), origin, direction, distance, hitTriggers, layerMask, ignore);
    }

    std::vector<RaycastHit> PhysicsSystem::RaycastAll(World& world, const Float3& origin, const Float3& direction,
                                                      float distance, bool hitTriggers, uint32_t layerMask,
                                                      std::span<const Entity> ignore)
    {
        std::vector<JPH::BodyID> ids = ResolveIgnoreList(ignore);
        return RaycastAll(world, origin, direction, distance, hitTriggers, layerMask, ids);
    }

    std::vector<RaycastHit> PhysicsSystem::RaycastAll(const Float3& origin, const Float3& direction, float distance,
                                                      bool hitTriggers, uint32_t layerMask,
                                                      std::span<const Entity> ignore)
    {
        return RaycastAll(GScene.GetWorld(), origin, direction, distance, hitTriggers, layerMask, ignore);
    }

    RaycastHit PhysicsSystem::SphereCast(World& world, const Float3& origin, const Float3& direction, float radius,
                                         float distance, bool hitTriggers, uint32_t layerMask,
                                         std::span<const JPH::BodyID> ignore)
    {
        JPH::SphereShape sphere(radius);
        JPH::RShapeCast shapeCast = JPH::RShapeCast::sFromWorldTransform(
            &sphere, JPH::Vec3::sOne(), JPH::RMat44::sTranslation(ToRVec3(origin)),
            ToVec3(direction).Normalized() * distance);

        RaycastBodyFilter filter;
        filter.HitTriggers = hitTriggers;
        for (auto id: ignore)
            filter.Ignore.IgnoreBody(id);

        GameplayLayerMaskFilter layerFilter;
        layerFilter.Mask = layerMask;

        JPH::ShapeCastSettings settings;
        JPH::ClosestHitCollisionCollector<JPH::CastShapeCollector> collector;
        m_physicsSystem.GetNarrowPhaseQuery().
                CastShape(shapeCast, settings, ToRVec3(origin), collector, {}, layerFilter, filter);
        if (!collector.HadHit())
            return {};

        return MakeHit(world, collector.mHit, ToRVec3(origin), distance);
    }

    RaycastHit PhysicsSystem::SphereCast(World& world, const Float3& origin, const Float3& direction, float radius,
                                         float distance, bool hitTriggers, uint32_t layerMask,
                                         std::span<const Entity> ignore)
    {
        std::vector<JPH::BodyID> ids = ResolveIgnoreList(ignore);
        return SphereCast(world, origin, direction, radius, distance, hitTriggers, layerMask, ids);
    }

    RaycastHit PhysicsSystem::SphereCast(const Float3& origin, const Float3& direction, float radius, float distance,
                                         bool hitTriggers, uint32_t layerMask, std::span<const JPH::BodyID> ignore)
    {
        return SphereCast(GScene.GetWorld(), origin, direction, radius, distance, hitTriggers, layerMask, ignore);
    }

    RaycastHit PhysicsSystem::SphereCast(const Float3& origin, const Float3& direction, float radius, float distance,
                                         bool hitTriggers, uint32_t layerMask, std::span<const Entity> ignore)
    {
        std::vector<JPH::BodyID> ids = ResolveIgnoreList(ignore);
        return SphereCast(GScene.GetWorld(), origin, direction, radius, distance, hitTriggers, layerMask, ids);
    }

    std::vector<RaycastHit> PhysicsSystem::SphereCastAll(World& world, const Float3& origin, const Float3& direction,
                                                         float radius, float distance, bool hitTriggers,
                                                         uint32_t layerMask, std::span<const JPH::BodyID> ignore)
    {
        JPH::SphereShape sphere(radius);
        JPH::RShapeCast shapeCast = JPH::RShapeCast::sFromWorldTransform(
            &sphere, JPH::Vec3::sOne(), JPH::RMat44::sTranslation(ToRVec3(origin)),
            ToVec3(direction).Normalized() * distance);

        RaycastBodyFilter filter;
        filter.HitTriggers = hitTriggers;
        for (auto id: ignore)
            filter.Ignore.IgnoreBody(id);

        GameplayLayerMaskFilter layerFilter;
        layerFilter.Mask = layerMask;

        JPH::ShapeCastSettings settings;
        JPH::AllHitCollisionCollector<JPH::CastShapeCollector> collector;
        m_physicsSystem.GetNarrowPhaseQuery().
                CastShape(shapeCast, settings, ToRVec3(origin), collector, {}, layerFilter, filter);
        if (!collector.HadHit())
            return {};

        std::vector<RaycastHit> hits;
        hits.reserve(collector.mHits.size());
        for (auto& h: collector.mHits)
            hits.push_back(MakeHit(world, h, ToRVec3(origin), distance));

        return hits;
    }

    std::vector<RaycastHit> PhysicsSystem::SphereCastAll(World& world, const Float3& origin, const Float3& direction,
                                                         float radius, float distance, bool hitTriggers,
                                                         uint32_t layerMask, std::span<const Entity> ignore)
    {
        std::vector<JPH::BodyID> ids = ResolveIgnoreList(ignore);
        return SphereCastAll(world, origin, direction, radius, distance, hitTriggers, layerMask, ids);
    }

    std::vector<RaycastHit> PhysicsSystem::SphereCastAll(const Float3& origin, const Float3& direction, float radius,
                                                         float distance, bool hitTriggers, uint32_t layerMask,
                                                         std::span<const JPH::BodyID> ignore)
    {
        return SphereCastAll(GScene.GetWorld(), origin, direction, radius, distance, hitTriggers, layerMask, ignore);
    }

    std::vector<RaycastHit> PhysicsSystem::SphereCastAll(const Float3& origin, const Float3& direction, float radius,
                                                         float distance, bool hitTriggers, uint32_t layerMask,
                                                         std::span<const Entity> ignore)
    {
        std::vector<JPH::BodyID> ids = ResolveIgnoreList(ignore);
        return SphereCastAll(GScene.GetWorld(), origin, direction, radius, distance, hitTriggers, layerMask, ids);
    }

    RaycastHit PhysicsSystem::BoxCast(World& world, const Float3& origin, const Float3& direction,
                                      const Float3& halfExtent, float distance, const Quaternion& rotation,
                                      bool hitTriggers, uint32_t layerMask,
                                      std::span<const JPH::BodyID> ignore)
    {
        JPH::BoxShape box(ToVec3(halfExtent));
        JPH::RShapeCast boxCast = JPH::RShapeCast::sFromWorldTransform(
            &box, JPH::Vec3::sOne(), JPH::RMat44::sRotationTranslation(ToQuat(rotation), ToRVec3(origin)),
            ToVec3(direction).Normalized() * distance);

        RaycastBodyFilter filter;
        filter.HitTriggers = hitTriggers;
        for (auto id: ignore)
            filter.Ignore.IgnoreBody(id);

        GameplayLayerMaskFilter layerFilter;
        layerFilter.Mask = layerMask;

        JPH::ShapeCastSettings settings;
        JPH::ClosestHitCollisionCollector<JPH::CastShapeCollector> collector;
        m_physicsSystem.GetNarrowPhaseQuery().
                CastShape(boxCast, settings, ToRVec3(origin), collector, {}, layerFilter, filter);
        if (!collector.HadHit())
            return {};

        return MakeHit(world, collector.mHit, ToRVec3(origin), distance);
    }

    RaycastHit PhysicsSystem::BoxCast(World& world, const Float3& origin, const Float3& direction,
                                      const Float3& halfExtent, float distance, const Quaternion& rotation,
                                      bool hitTriggers, uint32_t layerMask,
                                      std::span<const Entity> ignore)
    {
        std::vector<JPH::BodyID> ids = ResolveIgnoreList(ignore);
        return BoxCast(world, origin, direction, halfExtent, distance, rotation, hitTriggers, layerMask, ids);
    }

    RaycastHit PhysicsSystem::BoxCast(const Float3& origin, const Float3& direction, const Float3& halfExtent,
                                      float distance, const Quaternion& rotation, bool hitTriggers,
                                      uint32_t layerMask, std::span<const JPH::BodyID> ignore)
    {
        return BoxCast(GScene.GetWorld(), origin, direction, halfExtent, distance, rotation, hitTriggers, layerMask,
                       ignore);
    }

    RaycastHit PhysicsSystem::BoxCast(const Float3& origin, const Float3& direction, const Float3& halfExtent,
                                      float distance, const Quaternion& rotation, bool hitTriggers,
                                      uint32_t layerMask, std::span<const Entity> ignore)
    {
        std::vector<JPH::BodyID> ids = ResolveIgnoreList(ignore);
        return BoxCast(GScene.GetWorld(), origin, direction, halfExtent, distance, rotation, hitTriggers, layerMask,
                       ids);
    }

    std::vector<RaycastHit> PhysicsSystem::BoxCastAll(World& world, const Float3& origin, const Float3& direction,
                                                      const Float3& halfExtent, float distance,
                                                      const Quaternion& rotation, bool hitTriggers,
                                                      uint32_t layerMask, std::span<const JPH::BodyID> ignore)
    {
        JPH::BoxShape box(ToVec3(halfExtent));
        JPH::RShapeCast boxCast = JPH::RShapeCast::sFromWorldTransform(
            &box, JPH::Vec3::sOne(), JPH::RMat44::sRotationTranslation(ToQuat(rotation), ToRVec3(origin)),
            ToVec3(direction).Normalized() * distance);

        RaycastBodyFilter filter;
        filter.HitTriggers = hitTriggers;
        for (auto id: ignore)
            filter.Ignore.IgnoreBody(id);

        GameplayLayerMaskFilter layerFilter;
        layerFilter.Mask = layerMask;

        JPH::ShapeCastSettings settings;
        JPH::AllHitCollisionCollector<JPH::CastShapeCollector> collector;
        m_physicsSystem.GetNarrowPhaseQuery().
                CastShape(boxCast, settings, ToRVec3(origin), collector, {}, layerFilter, filter);
        if (!collector.HadHit())
            return {};

        std::vector<RaycastHit> hits;
        hits.reserve(collector.mHits.size());
        for (auto& h: collector.mHits)
            hits.push_back(MakeHit(world, h, ToRVec3(origin), distance));

        return hits;
    }

    std::vector<RaycastHit> PhysicsSystem::BoxCastAll(World& world, const Float3& origin, const Float3& direction,
                                                      const Float3& halfExtent, float distance,
                                                      const Quaternion& rotation, bool hitTriggers,
                                                      uint32_t layerMask, std::span<const Entity> ignore)
    {
        std::vector<JPH::BodyID> ids = ResolveIgnoreList(ignore);
        return BoxCastAll(world, origin, direction, halfExtent, distance, rotation, hitTriggers, layerMask, ids);
    }

    std::vector<RaycastHit> PhysicsSystem::BoxCastAll(const Float3& origin, const Float3& direction,
                                                      const Float3& halfExtent, float distance,
                                                      const Quaternion& rotation, bool hitTriggers,
                                                      uint32_t layerMask, std::span<const JPH::BodyID> ignore)
    {
        return BoxCastAll(GScene.GetWorld(), origin, direction, halfExtent, distance, rotation, hitTriggers,
                          layerMask, ignore);
    }

    std::vector<RaycastHit> PhysicsSystem::BoxCastAll(const Float3& origin, const Float3& direction,
                                                      const Float3& halfExtent, float distance,
                                                      const Quaternion& rotation, bool hitTriggers,
                                                      uint32_t layerMask, std::span<const Entity> ignore)
    {
        std::vector<JPH::BodyID> ids = ResolveIgnoreList(ignore);
        return BoxCastAll(GScene.GetWorld(), origin, direction, halfExtent, distance, rotation, hitTriggers,
                          layerMask, ids);
    }

    std::vector<OverlapHit> PhysicsSystem::OverlapSphere(World& world, const Float3& center, float radius,
                                                         bool hitTriggers, uint32_t layerMask,
                                                         std::span<const JPH::BodyID> ignore)
    {
        JPH::SphereShape sphere(radius);

        RaycastBodyFilter filter;
        filter.HitTriggers = hitTriggers;
        for (auto id: ignore)
            filter.Ignore.IgnoreBody(id);

        GameplayLayerMaskFilter layerFilter;
        layerFilter.Mask = layerMask;

        JPH::CollideShapeSettings settings;
        JPH::AllHitCollisionCollector<JPH::CollideShapeCollector> collector;
        m_physicsSystem.GetNarrowPhaseQuery().
                CollideShape(&sphere, JPH::Vec3::sOne(), JPH::RMat44::sTranslation(ToRVec3(center)), settings,
                             ToRVec3(center), collector, {}, layerFilter, filter);

        std::vector<OverlapHit> hits;
        hits.reserve(collector.mHits.size());
        for (auto& h: collector.mHits)
            hits.push_back(MakeOverlapHit(world, h));

        return hits;
    }

    std::vector<OverlapHit> PhysicsSystem::OverlapSphere(World& world, const Float3& center, float radius,
                                                         bool hitTriggers, uint32_t layerMask,
                                                         std::span<const Entity> ignore)
    {
        std::vector<JPH::BodyID> ids = ResolveIgnoreList(ignore);
        return OverlapSphere(world, center, radius, hitTriggers, layerMask, ids);
    }

    std::vector<OverlapHit> PhysicsSystem::OverlapSphere(const Float3& center, float radius, bool hitTriggers,
                                                         uint32_t layerMask, std::span<const JPH::BodyID> ignore)
    {
        return OverlapSphere(GScene.GetWorld(), center, radius, hitTriggers, layerMask, ignore);
    }

    std::vector<OverlapHit> PhysicsSystem::OverlapSphere(const Float3& center, float radius, bool hitTriggers,
                                                         uint32_t layerMask, std::span<const Entity> ignore)
    {
        std::vector<JPH::BodyID> ids = ResolveIgnoreList(ignore);
        return OverlapSphere(GScene.GetWorld(), center, radius, hitTriggers, layerMask, ids);
    }

    std::vector<OverlapHit> PhysicsSystem::OverlapBox(World& world, const Float3& center, const Float3& halfExtent,
                                                      const Quaternion& rotation, bool hitTriggers,
                                                      uint32_t layerMask, std::span<const JPH::BodyID> ignore)
    {
        JPH::BoxShape box(ToVec3(halfExtent));

        RaycastBodyFilter filter;
        filter.HitTriggers = hitTriggers;
        for (auto id: ignore)
            filter.Ignore.IgnoreBody(id);

        GameplayLayerMaskFilter layerFilter;
        layerFilter.Mask = layerMask;

        JPH::CollideShapeSettings settings;
        JPH::AllHitCollisionCollector<JPH::CollideShapeCollector> collector;
        m_physicsSystem.GetNarrowPhaseQuery().
                CollideShape(&box, JPH::Vec3::sOne(),
                             JPH::RMat44::sRotationTranslation(ToQuat(rotation), ToRVec3(center)),
                             settings, ToRVec3(center), collector, {}, layerFilter, filter);

        std::vector<OverlapHit> hits;
        hits.reserve(collector.mHits.size());
        for (auto& h: collector.mHits)
            hits.push_back(MakeOverlapHit(world, h));

        return hits;
    }

    std::vector<OverlapHit> PhysicsSystem::OverlapBox(World& world, const Float3& center, const Float3& halfExtent,
                                                      const Quaternion& rotation, bool hitTriggers,
                                                      uint32_t layerMask, std::span<const Entity> ignore)
    {
        std::vector<JPH::BodyID> ids = ResolveIgnoreList(ignore);
        return OverlapBox(world, center, halfExtent, rotation, hitTriggers, layerMask, ids);
    }

    std::vector<OverlapHit> PhysicsSystem::OverlapBox(const Float3& center, const Float3& halfExtent,
                                                      const Quaternion& rotation, bool hitTriggers,
                                                      uint32_t layerMask, std::span<const JPH::BodyID> ignore)
    {
        return OverlapBox(GScene.GetWorld(), center, halfExtent, rotation, hitTriggers, layerMask, ignore);
    }

    std::vector<OverlapHit> PhysicsSystem::OverlapBox(const Float3& center, const Float3& halfExtent,
                                                      const Quaternion& rotation, bool hitTriggers,
                                                      uint32_t layerMask, std::span<const Entity> ignore)
    {
        std::vector<JPH::BodyID> ids = ResolveIgnoreList(ignore);
        return OverlapBox(GScene.GetWorld(), center, halfExtent, rotation, hitTriggers, layerMask, ids);
    }

    std::vector<OverlapHit> PhysicsSystem::OverlapCapsule(World& world, const Float3& center, float radius,
                                                          float halfHeight, const Quaternion& rotation,
                                                          bool hitTriggers, uint32_t layerMask,
                                                          std::span<const JPH::BodyID> ignore)
    {
        JPH::CapsuleShape capsule(halfHeight, radius);

        RaycastBodyFilter filter;
        filter.HitTriggers = hitTriggers;
        for (auto id: ignore)
            filter.Ignore.IgnoreBody(id);

        GameplayLayerMaskFilter layerFilter;
        layerFilter.Mask = layerMask;

        JPH::CollideShapeSettings settings;
        JPH::AllHitCollisionCollector<JPH::CollideShapeCollector> collector;
        m_physicsSystem.GetNarrowPhaseQuery().
                CollideShape(&capsule, JPH::Vec3::sOne(),
                             JPH::RMat44::sRotationTranslation(ToQuat(rotation), ToRVec3(center)),
                             settings, ToRVec3(center), collector, {}, layerFilter, filter);

        std::vector<OverlapHit> hits;
        hits.reserve(collector.mHits.size());
        for (auto& h: collector.mHits)
            hits.push_back(MakeOverlapHit(world, h));

        return hits;
    }

    std::vector<OverlapHit> PhysicsSystem::OverlapCapsule(World& world, const Float3& center, float radius,
                                                          float halfHeight, const Quaternion& rotation,
                                                          bool hitTriggers, uint32_t layerMask,
                                                          std::span<const Entity> ignore)
    {
        std::vector<JPH::BodyID> ids = ResolveIgnoreList(ignore);
        return OverlapCapsule(world, center, radius, halfHeight, rotation, hitTriggers, layerMask, ids);
    }

    std::vector<OverlapHit> PhysicsSystem::OverlapCapsule(const Float3& center, float radius, float halfHeight,
                                                          const Quaternion& rotation, bool hitTriggers,
                                                          uint32_t layerMask, std::span<const JPH::BodyID> ignore)
    {
        return OverlapCapsule(GScene.GetWorld(), center, radius, halfHeight, rotation, hitTriggers, layerMask,
                              ignore);
    }

    std::vector<OverlapHit> PhysicsSystem::OverlapCapsule(const Float3& center, float radius, float halfHeight,
                                                          const Quaternion& rotation, bool hitTriggers,
                                                          uint32_t layerMask, std::span<const Entity> ignore)
    {
        std::vector<JPH::BodyID> ids = ResolveIgnoreList(ignore);
        return OverlapCapsule(GScene.GetWorld(), center, radius, halfHeight, rotation, hitTriggers, layerMask, ids);
    }

    OverlapHit PhysicsSystem::MakeOverlapHit(World& world, const JPH::CollideShapeResult& result)
    {
        auto it = m_bodyInfo.find(result.mBodyID2);
        if (it == m_bodyInfo.end())
            return {};

        OverlapHit hit;
        hit.BodyID = result.mBodyID2;
        hit.Target = world.entity(it->second.Entity);
        hit.Point = ToFloat3(result.mContactPointOn2);
        hit.Normal = ToFloat3(result.mPenetrationAxis.Normalized());
        hit.PenetrationDepth = result.mPenetrationDepth;
        return hit;
    }

    RaycastHit PhysicsSystem::MakeHit(World& world, JPH::BodyID id, float fraction, const JPH::RRayCast& ray,
                                      JPH::SubShapeID subShape)
    {
        auto it = m_bodyInfo.find(id);
        if (it == m_bodyInfo.end())
            return {};

        JPH::RVec3 point = ray.GetPointOnRay(fraction);

        JPH::BodyLockRead lock(m_physicsSystem.GetBodyLockInterface(), id);
        JPH::Vec3 normal = lock.Succeeded()
                               ? lock.GetBody().GetWorldSpaceSurfaceNormal(subShape, point)
                               : JPH::Vec3::sZero();

        RaycastHit hit;
        hit.Hit = true;
        hit.BodyID = id;
        hit.Target = world.entity(it->second.Entity);
        hit.Point = ToFloat3(point);
        hit.Normal = ToFloat3(normal);
        hit.Distance = fraction * ray.mDirection.Length();
        return hit;
    }

    RaycastHit PhysicsSystem::MakeHit(World& world, JPH::ShapeCastResult& result, JPH::RVec3Arg baseOffset,
                                      float castLength)
    {
        auto it = m_bodyInfo.find(result.mBodyID2);
        if (it == m_bodyInfo.end())
            return {};

        JPH::RVec3 point = baseOffset + JPH::RVec3(result.mContactPointOn2);

        JPH::BodyLockRead lock(m_physicsSystem.GetBodyLockInterface(), result.mBodyID2);
        JPH::Vec3 normal = lock.Succeeded()
                               ? lock.GetBody().GetWorldSpaceSurfaceNormal(result.mSubShapeID2, point)
                               : JPH::Vec3::sZero();

        RaycastHit hit;
        hit.Hit = true;
        hit.BodyID = result.mBodyID2;
        hit.Target = world.entity(it->second.Entity);
        hit.Point = ToFloat3(point);
        hit.Normal = ToFloat3(normal);
        hit.Distance = result.mFraction * castLength;
        return hit;
    }

    void PhysicsSystem::QueueContact(JPH::BodyID id1, JPH::BodyID id2, JPH::RVec3 point, JPH::Vec3 normal,
                                     EContactPhase phase)
    {
        std::lock_guard lock(m_contactMutex);
        m_pendingContacts.push_back({id1, id2, point, normal, phase});
    }

    void PhysicsSystem::ApplyPendingForces()
    {
        auto& bi = GetBodyInterface();
        for (auto& cmd: m_pendingForceCommands)
        {
            switch (cmd.Type)
            {
                case EForceCommand::Force:
                    bi.AddForce(cmd.BodyID, cmd.Value);
                    break;
                case EForceCommand::ForceAtPosition:
                    bi.AddForce(cmd.BodyID, cmd.Value, cmd.Position);
                    break;
                case EForceCommand::Torque:
                    bi.AddTorque(cmd.BodyID, cmd.Value);
                    break;
                case EForceCommand::Impulse:
                    bi.AddImpulse(cmd.BodyID, cmd.Value);
                    break;
                case EForceCommand::ImpulseAtPosition:
                    bi.AddImpulse(cmd.BodyID, cmd.Value, cmd.Position);
                    break;
                case EForceCommand::AngularImpulse:
                    bi.AddAngularImpulse(cmd.BodyID, cmd.Value);
                    break;
            }
        }

        m_pendingForceCommands.clear();
    }

    uint64_t PhysicsSystem::MakeContactKey(JPH::BodyID id1, JPH::BodyID id2)
    {
        uint32_t a = id1.GetIndexAndSequenceNumber(), b = id2.GetIndexAndSequenceNumber();
        if (a > b)
            std::swap(a, b);
        return (uint64_t(a) << 32) | b;
    }

    nlohmann::json PhysicsSystem::LoadPhysicsConfigJson(const std::string& fullPath)
    {
        std::ifstream stream(fullPath);
        if (!stream)
        {
            LogError("Physics config file not found: '{}'. Fall back to defaults.", fullPath);
            return {};
        }

        try
        {
            nlohmann::json json;
            stream >> json;
            return json;
        }
        catch (const nlohmann::json::exception& e)
        {
            LogError("Physics config parse error '{}': {} - falling back to defaults", fullPath, e.what());
            return {};
        }
    }

    void PhysicsSystem::BroadcastCollisionEnter(Entity e1, Entity e2,
        const Events::CollisionEnterEvent& evt1, const Events::CollisionEnterEvent& evt2)
    {
        if (auto p1 = e1.get_ref<RigidbodyComponent>())
            p1->OnCollisionEnter.Broadcast({}, evt1);
        if (auto p2 = e2.get_ref<RigidbodyComponent>())
            p2->OnCollisionEnter.Broadcast({}, evt2);
        if (auto c1 = e1.get_ref<CharacterControllerComponent>())
            c1->OnCollisionEnter.Broadcast({}, evt1);
        if (auto c2 = e2.get_ref<CharacterControllerComponent>())
            c2->OnCollisionEnter.Broadcast({}, evt2);
    }

    void PhysicsSystem::BroadcastCollisionStay(Entity e1, Entity e2,
        const Events::CollisionStayEvent& evt1, const Events::CollisionStayEvent& evt2)
    {
        if (auto p1 = e1.get_ref<RigidbodyComponent>())
            p1->OnCollisionStay.Broadcast({}, evt1);
        if (auto p2 = e2.get_ref<RigidbodyComponent>())
            p2->OnCollisionStay.Broadcast({}, evt2);
        if (auto c1 = e1.get_ref<CharacterControllerComponent>())
            c1->OnCollisionStay.Broadcast({}, evt1);
        if (auto c2 = e2.get_ref<CharacterControllerComponent>())
            c2->OnCollisionStay.Broadcast({}, evt2);
    }

    void PhysicsSystem::BroadcastCollisionExit(Entity e1, Entity e2,
        const Events::CollisionExitEvent& evt1, const Events::CollisionExitEvent& evt2)
    {
        if (auto p1 = e1.get_ref<RigidbodyComponent>())
            p1->OnCollisionExit.Broadcast({}, evt1);
        if (auto p2 = e2.get_ref<RigidbodyComponent>())
            p2->OnCollisionExit.Broadcast({}, evt2);
        if (auto c1 = e1.get_ref<CharacterControllerComponent>())
            c1->OnCollisionExit.Broadcast({}, evt1);
        if (auto c2 = e2.get_ref<CharacterControllerComponent>())
            c2->OnCollisionExit.Broadcast({}, evt2);
    }

    void PhysicsSystem::BroadcastTriggerEnter(Entity e1, Entity e2,
        const Events::TriggerEnterEvent& evt1, const Events::TriggerEnterEvent& evt2)
    {
        if (auto p1 = e1.get_ref<RigidbodyComponent>())
            p1->OnTriggerEnter.Broadcast({}, evt1);
        if (auto p2 = e2.get_ref<RigidbodyComponent>())
            p2->OnTriggerEnter.Broadcast({}, evt2);
        if (auto c1 = e1.get_ref<CharacterControllerComponent>())
            c1->OnTriggerEnter.Broadcast({}, evt1);
        if (auto c2 = e2.get_ref<CharacterControllerComponent>())
            c2->OnTriggerEnter.Broadcast({}, evt2);
    }

    void PhysicsSystem::BroadcastTriggerStay(Entity e1, Entity e2,
        const Events::TriggerStayEvent& evt1, const Events::TriggerStayEvent& evt2)
    {
        if (auto p1 = e1.get_ref<RigidbodyComponent>())
            p1->OnTriggerStay.Broadcast({}, evt1);
        if (auto p2 = e2.get_ref<RigidbodyComponent>())
            p2->OnTriggerStay.Broadcast({}, evt2);
        if (auto c1 = e1.get_ref<CharacterControllerComponent>())
            c1->OnTriggerStay.Broadcast({}, evt1);
        if (auto c2 = e2.get_ref<CharacterControllerComponent>())
            c2->OnTriggerStay.Broadcast({}, evt2);
    }

    void PhysicsSystem::BroadcastTriggerExit(Entity e1, Entity e2,
        const Events::TriggerExitEvent& evt1, const Events::TriggerExitEvent& evt2)
    {
        if (auto p1 = e1.get_ref<RigidbodyComponent>())
            p1->OnTriggerExit.Broadcast({}, evt1);
        if (auto p2 = e2.get_ref<RigidbodyComponent>())
            p2->OnTriggerExit.Broadcast({}, evt2);
        if (auto c1 = e1.get_ref<CharacterControllerComponent>())
            c1->OnTriggerExit.Broadcast({}, evt1);
        if (auto c2 = e2.get_ref<CharacterControllerComponent>())
            c2->OnTriggerExit.Broadcast({}, evt2);
    }

    void PhysicsSystem::EmitStay(::World& world, const PendingContact& contact)
    {
        auto it1 = m_bodyInfo.find(contact.Body1);
        auto it2 = m_bodyInfo.find(contact.Body2);
        if (it1 == m_bodyInfo.end() || it2 == m_bodyInfo.end())
            return;

        bool isTrigger = it1->second.IsTrigger || it2->second.IsTrigger;
        Entity e1 = world.entity(it1->second.Entity);
        Entity e2 = world.entity(it2->second.Entity);
        Float3 point = ToFloat3(contact.Point), normal = ToFloat3(contact.Normal);

        if (isTrigger)
        {
            Events::TriggerStayEvent evt1{e1, e2, point, normal}, evt2{e2, e1, point, normal};
            GEventBus.Emit(Events::OnTriggerStay, evt1, EmitBadge<PhysicsSystem>{});
            BroadcastTriggerStay(e1, e2, evt1, evt2);
        }
        else
        {
            Events::CollisionStayEvent evt1{e1, e2, point, normal}, evt2{e2, e1, point, normal};
            GEventBus.Emit(Events::OnCollisionStay, evt1, EmitBadge<PhysicsSystem>{});
            BroadcastCollisionStay(e1, e2, evt1, evt2);
        }
    }

    void PhysicsSystem::DispatchContact(::World& world, const PendingContact& contact)
    {
        auto it1 = m_bodyInfo.find(contact.Body1);
        auto it2 = m_bodyInfo.find(contact.Body2);

        if (it1 == m_bodyInfo.end() || it2 == m_bodyInfo.end())
            return;

        bool isTrigger = it1->second.IsTrigger || it2->second.IsTrigger;
        Entity e1 = world.entity(it1->second.Entity);
        Entity e2 = world.entity(it2->second.Entity);

        switch (contact.Phase)
        {
            case EContactPhase::Enter:
            {
                Float3 point = ToFloat3(contact.Point), normal = ToFloat3(contact.Normal);
                if (isTrigger)
                {
                    Events::TriggerEnterEvent evt1{e1, e2, point, normal}, evt2{e2, e1, point, normal};
                    GEventBus.Emit(Events::OnTriggerEnter, evt1, EmitBadge<PhysicsSystem>{});
                    BroadcastTriggerEnter(e1, e2, evt1, evt2);
                }
                else
                {
                    Events::CollisionEnterEvent evt1{e1, e2, point, normal}, evt2{e2, e1, point, normal};
                    GEventBus.Emit(Events::OnCollisionEnter, evt1, EmitBadge<PhysicsSystem>{});
                    BroadcastCollisionEnter(e1, e2, evt1, evt2);
                }
                m_activeContacts[MakeContactKey(contact.Body1, contact.Body2)] = contact;
                break;
            }
            case EContactPhase::Stay:
            {
                // Jolt stops calling OnContactPersisted once a touching pair sleeps; the
                // Stay event itself is re-emitted every tick from m_activeContacts in
                // Update(), so this just refreshes the cached point/normal while awake.
                m_activeContacts[MakeContactKey(contact.Body1, contact.Body2)] = contact;
                break;
            }
            case EContactPhase::Exit:
            {
                auto& bi = GetBodyInterface();
                if (!bi.IsActive(contact.Body1) && !bi.IsActive(contact.Body2))
                    break;

                m_activeContacts.erase(MakeContactKey(contact.Body1, contact.Body2));

                if (isTrigger)
                {
                    Events::TriggerExitEvent evt1{e1, e2}, evt2{e2, e1};
                    GEventBus.Emit(Events::OnTriggerExit, evt1, EmitBadge<PhysicsSystem>{});
                    BroadcastTriggerExit(e1, e2, evt1, evt2);
                }
                else
                {
                    Events::CollisionExitEvent evt1{e1, e2}, evt2{e2, e1};
                    GEventBus.Emit(Events::OnCollisionExit, evt1, EmitBadge<PhysicsSystem>{});
                    BroadcastCollisionExit(e1, e2, evt1, evt2);
                }
                break;
            }
        }
    }

    void PhysicsSystem::Initialize()
    {
        JPH::RegisterDefaultAllocator();
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();

        nlohmann::json config = LoadPhysicsConfigJson(GetAbsolutePath("Assets/Config/Physics.json"));

        m_tempAllocator = std::make_unique<
            JPH::TempAllocatorImpl>(config.value("TempAllocatorBytes", 10 * 1024 * 1024));

        int jobThreads = config.value("JobThreads", -1);
        if (jobThreads <= 0)
            jobThreads = (int) std::thread::hardware_concurrency() - 1;

        m_jobSystem = std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers,
                                                                 jobThreads);

        m_collisionSteps = config.value("CollisionSteps", 1);

        JPH::uint maxBodies = config.value("MaxBodies", 1024u);
        JPH::uint numBodyMutexes = config.value("NumBodyMutexes", 0);
        JPH::uint maxBodyPairs = config.value("MaxBodyPairs", 1024u);
        JPH::uint maxContactConstraints = config.value("MaxContactConstraints", 1024u);

        m_layers.Build(config);

        m_physicsSystem.Init(maxBodies, numBodyMutexes, maxBodyPairs, maxContactConstraints,
                             m_layers.GetBroadPhaseLayerInterface(), m_layers.GetObjectVsBroadPhaseLayerFilter(),
                             m_layers.GetPairFilter());

        auto g = config.value("Gravity", std::vector<float>{0.0f, -9.81f, 0.0f});
        m_physicsSystem.SetGravity(JPH::Vec3(g[0], g[1], g[2]));

        JPH::PhysicsSettings settings = m_physicsSystem.GetPhysicsSettings();
        settings.mBaumgarte = config.value("Baumgarte", 0.2f);
        settings.mPenetrationSlop = config.value("PenetrationSlop", 0.02f);
        settings.mSpeculativeContactDistance = config.value("SpeculativeContactDistance", 0.02f);
        settings.mNumVelocitySteps = config.value("NumVelocitySteps", 10u);
        settings.mNumPositionSteps = config.value("NumPositionSteps", 2u);
        m_physicsSystem.SetPhysicsSettings(settings);

        m_contactListener.Owner = this;
        m_physicsSystem.SetContactListener(&m_contactListener);
    }

    void PhysicsSystem::Update(flecs::world& world, float deltaTime)
    {
        world.query<RigidbodyComponent, TransformComponent>().each(
            [&](RigidbodyComponent& physics, TransformComponent& transform)
            {
                if (physics.MotionType == EMotionType::Static || physics.BodyID.IsInvalid())
                {
                    return;
                }

                auto& bi = m_physicsSystem.GetBodyInterface();
                bi.SetLinearVelocity(physics.BodyID, ToVec3(physics.Velocity));
                bi.SetAngularVelocity(physics.BodyID, ToVec3(physics.AngularVelocity));

                constexpr EPhysicsDOF RotationDOFs =
                        EPhysicsDOF::RotationX | EPhysicsDOF::RotationY | EPhysicsDOF::RotationZ;

                bool physicsOwnsRotation = physics.MotionType == EMotionType::Dynamic
                                           && (static_cast<uint8_t>(physics.AllowedDOFs) & static_cast<uint8_t>(
                                                   RotationDOFs)) == static_cast<uint8_t>(RotationDOFs);

                if (!physicsOwnsRotation)
                    bi.SetRotation(physics.BodyID, ToQuat(transform.Rotation), JPH::EActivation::DontActivate);
            });

        ApplyPendingForces();

        m_physicsSystem.Update(deltaTime, m_collisionSteps, m_tempAllocator.get(), m_jobSystem.get());

        std::vector<PendingContact> contacts = {}; {
            std::lock_guard<std::mutex> lock(m_contactMutex);
            contacts.swap(m_pendingContacts);
        }

        for (auto& c: contacts)
        {
            DispatchContact(world, c);
        }

        for (auto& [key, c]: m_activeContacts)
        {
            EmitStay(world, c);
        }

        world.query<RigidbodyComponent, TransformComponent>().each(
            [&](RigidbodyComponent& physics, TransformComponent& transform)
            {
                if (physics.MotionType == EMotionType::Static || physics.BodyID.IsInvalid())
                {
                    return;
                }

                auto& bi = m_physicsSystem.GetBodyInterface();
                transform.Position = ToFloat3(bi.GetPosition(physics.BodyID));
                transform.Rotation = ToQuaternion(bi.GetRotation(physics.BodyID));
                physics.Velocity = ToFloat3(bi.GetLinearVelocity(physics.BodyID));
                physics.AngularVelocity = ToFloat3(bi.GetAngularVelocity(physics.BodyID));
            });

        world.query<CharacterControllerComponent, TransformComponent>().each(
            [&](CharacterControllerComponent& controller, TransformComponent& transform)
            {
                if (!controller.Controller)
                    return;

                controller.Controller->SetRotation(transform.Rotation);
                controller.Controller->Update(deltaTime);
                transform.Position = controller.Controller->GetPosition();
                transform.Rotation = controller.Controller->GetRotation();
            });
    }

    void PhysicsSystem::Shutdown()
    {
        JPH::UnregisterTypes();
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;
    }

    JPH::BodyInterface& PhysicsSystem::GetBodyInterface()
    {
        return m_physicsSystem.GetBodyInterface();
    }
} // RTGDEngine
