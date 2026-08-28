//
// Created by ivan on 8/28/26.
//

#include "Systems/Physics/PhysicalCharacterController.h"

namespace RTGDEngine
{
    PhysicalCharacterController::PhysicalCharacterController(const Float3& position, const Quaternion& rotation,
        JPH::ShapeRefC shape, float mass, float friction, float maxSlopeAngleDeg, uint8_t layer)
    {
    }

    Float3 PhysicalCharacterController::GetGroundNormal() const
    {
    }

    ECharacterGroundState PhysicalCharacterController::GetGroundState() const
    {
    }

    Float3 PhysicalCharacterController::GetGroundVelocity() const
    {
    }

    Float3 PhysicalCharacterController::GetLinearVelocity() const
    {
    }

    Float3 PhysicalCharacterController::GetPosition() const
    {
    }

    Quaternion PhysicalCharacterController::GetRotation() const
    {
    }

    bool PhysicalCharacterController::IsGrounded() const
    {
    }

    void PhysicalCharacterController::Jump(float jumpSpeed)
    {
    }

    void PhysicalCharacterController::SetLinearVelocity(const Float3 velocity)
    {
    }

    void PhysicalCharacterController::SetPosition(const Float3 position)
    {
    }

    void PhysicalCharacterController::SetRotation(const Quaternion rotation)
    {
    }

    void PhysicalCharacterController::Update(float deltaTime)
    {
    }
}
