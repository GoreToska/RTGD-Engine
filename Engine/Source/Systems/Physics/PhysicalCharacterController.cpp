//
// Created by ivan on 8/28/26.
//

#include "Systems/Physics/PhysicalCharacterController.h"
#include "Tools/Alias.h"
#include <Jolt/Physics/Character/Character.h>

#include "Systems/Physics/PhysicsLayer.h"
#include "Systems/Physics/PhysicsSystem.h"
#include "Tools/JoltConversions.h"

namespace RTGDEngine
{
    PhysicalCharacterController::PhysicalCharacterController(const Float3& position, const Quaternion& rotation,
                                                             JPH::ShapeRefC shape, float mass, float friction,
                                                             float maxSlopeAngleDeg, uint8_t layer, Entity entity)
        : m_mass(mass)
    {
        JPH::CharacterSettings settings;
        settings.mShape = shape;
        settings.mFriction = friction;
        settings.mMass = mass;
        settings.mMaxSlopeAngle = JPH::DegreesToRadians(maxSlopeAngleDeg);
        settings.mLayer = Layers::Encode(layer, true);

        m_character = new JPH::Character(&settings, ToRVec3(position), ToQuat(rotation), 0, &GPhysics.GetJoltSystem());
        m_character->AddToPhysicsSystem(JPH::EActivation::Activate);
        GPhysics.RegisterBody(m_character->GetBodyID(), entity.id(), false);
    }

    PhysicalCharacterController::~PhysicalCharacterController()
    {
        GPhysics.UnregisterBody(m_character->GetBodyID());
        m_character->RemoveFromPhysicsSystem();
    }

    JPH::BodyID PhysicalCharacterController::GetBodyID() const
    {
        return m_character->GetBodyID();
    }

    Float3 PhysicalCharacterController::GetGroundNormal() const
    {
        return ToFloat3(m_character->GetGroundNormal());
    }

    ECharacterGroundState PhysicalCharacterController::GetGroundState() const
    {
        return static_cast<ECharacterGroundState>(m_character->GetGroundState());
    }

    Float3 PhysicalCharacterController::GetGroundVelocity() const
    {
        return ToFloat3(m_character->GetGroundVelocity());
    }

    Float3 PhysicalCharacterController::GetLinearVelocity() const
    {
        return ToFloat3(m_character->GetLinearVelocity());
    }

    Float3 PhysicalCharacterController::GetPosition() const
    {
        return ToFloat3(m_character->GetPosition());
    }

    Quaternion PhysicalCharacterController::GetRotation() const
    {
        return ToQuaternion(m_character->GetRotation());
    }

    bool PhysicalCharacterController::IsGrounded() const
    {
        return m_character->IsSupported();
    }

    void PhysicalCharacterController::Jump(float jumpSpeed)
    {
        JPH::Vec3 v = m_character->GetLinearVelocity();
        v.SetY(jumpSpeed);
        m_character->SetLinearVelocity(v);
    }

    void PhysicalCharacterController::SetLinearVelocity(const Float3 velocity)
    {
        m_character->SetLinearVelocity(ToVec3(velocity));
    }

    void PhysicalCharacterController::SetPosition(const Float3 position)
    {
        m_character->SetPosition(ToRVec3(position));
    }

    void PhysicalCharacterController::SetRotation(const Quaternion rotation)
    {
        m_character->SetRotation(ToQuat(rotation));
    }

    void PhysicalCharacterController::Update(float deltaTime)
    {
        m_character->PostSimulation(PhysicsSystem::kCharacterGroundTolerance);
    }
}
