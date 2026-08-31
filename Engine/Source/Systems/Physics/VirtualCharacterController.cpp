//
// Created by ivan on 8/28/26.
//

#include "Systems/Physics/VirtualCharacterController.h"

#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Systems/Physics/PhysicsLayer.h>

#include "Systems/Physics/PhysicsSystem.h"
#include "Tools/JoltConversions.h"

namespace RTGDEngine
{
    VirtualCharacterController::VirtualCharacterController(const Float3& position, const Quaternion& rotation,
                                                           JPH::ShapeRefC shape, float mass, float maxStrength,
                                                           float maxSlopeAngleDeg, uint8_t layer, Entity entity)
        : m_objectLayer(Layers::Encode(layer, true))
    {
        JPH::CharacterVirtualSettings settings;
        settings.mShape = shape;
        settings.mMass = mass;
        settings.mMaxSlopeAngle = JPH::DegreesToRadians(maxSlopeAngleDeg);
        settings.mMaxStrength = maxStrength;
        settings.mInnerBodyShape = shape;
        settings.mInnerBodyLayer = layer;

        m_character = new JPH::CharacterVirtual(&settings, ToRVec3(position), ToQuat(rotation), 0,
                                                &GPhysics.GetJoltSystem());

        GPhysics.RegisterBody(m_character->GetInnerBodyID(), entity.id(), false);
    }

    VirtualCharacterController::~VirtualCharacterController()
    {
        GPhysics.UnregisterBody(m_character->GetInnerBodyID());
    }

    JPH::BodyID VirtualCharacterController::GetBodyID() const
    {
        return m_character->GetInnerBodyID();
    }

    Float3 VirtualCharacterController::GetGroundNormal() const
    {
        return ToFloat3(m_character->GetGroundNormal());
    }

    ECharacterGroundState VirtualCharacterController::GetGroundState() const
    {
        return static_cast<ECharacterGroundState>(m_character->GetGroundState());
    }

    Float3 VirtualCharacterController::GetGroundVelocity() const
    {
        return ToFloat3(m_character->GetGroundVelocity());
    }

    Float3 VirtualCharacterController::GetLinearVelocity() const
    {
        return ToFloat3(m_character->GetLinearVelocity());
    }

    Float3 VirtualCharacterController::GetPosition() const
    {
        return ToFloat3(m_character->GetPosition());
    }

    Quaternion VirtualCharacterController::GetRotation() const
    {
        return ToQuaternion(m_character->GetRotation());
    }

    bool VirtualCharacterController::IsGrounded() const
    {
        return m_character->IsSupported();
    }

    void VirtualCharacterController::Jump(float jumpSpeed)
    {
        JPH::Vec3 v = m_character->GetLinearVelocity();
        v.SetY(jumpSpeed);
        m_character->SetLinearVelocity(v);
    }

    void VirtualCharacterController::SetLinearVelocity(const Float3 velocity)
    {
        m_character->SetLinearVelocity(ToVec3(velocity));
    }

    void VirtualCharacterController::SetPosition(const Float3 position)
    {
        m_character->SetPosition(ToRVec3(position));
    }

    void VirtualCharacterController::SetRotation(const Quaternion rotation)
    {
        m_character->SetRotation(ToQuat(rotation));
    }

    void VirtualCharacterController::Update(float deltaTime)
    {
        JPH::PhysicsSystem& system = GPhysics.GetJoltSystem();
        JPH::Vec3 gravity = system.GetGravity();
        JPH::Vec3 up = m_character->GetUp();

        JPH::Vec3 velocity = m_character->GetLinearVelocity();
        JPH::Vec3 verticalVelocity = up * velocity.Dot(up);
        JPH::Vec3 horizontalVelocity = velocity - verticalVelocity;

        if (m_character->GetGroundState() == JPH::CharacterVirtual::EGroundState::OnGround
            && verticalVelocity.Dot(up) < 0.1f)
        {
            verticalVelocity = up * m_character->GetGroundVelocity().Dot(up);
        }

        verticalVelocity += gravity * deltaTime;
        m_character->SetLinearVelocity(horizontalVelocity + verticalVelocity);

        m_character->UpdateGroundVelocity();
        m_character->Update(deltaTime, gravity, system.GetDefaultBroadPhaseLayerFilter(m_objectLayer),
                            system.GetDefaultLayerFilter(m_objectLayer), JPH::BodyFilter{}, JPH::ShapeFilter{},
                            GPhysics.GetTempAllocator());
    }
}
