//
// Created by ivan on 8/28/26.
//

#pragma once
#include "ICharacterController.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>

#include "Jolt/Physics/Collision/ObjectLayer.h"

namespace JPH
{
    class CharacterVirtual;
    class Character;
}

namespace RTGDEngine
{
    class VirtualCharacterController : public ICharacterController
    {
    public:
        VirtualCharacterController(const Float3& position, const Quaternion& rotation, JPH::ShapeRefC shape,
                                   float mass, float maxStrength, float maxSlopeAngleDeg, uint8_t layer);

        ~VirtualCharacterController() override;

        Float3 GetGroundNormal() const override;

        ECharacterGroundState GetGroundState() const override;

        Float3 GetGroundVelocity() const override;

        Float3 GetLinearVelocity() const override;

        Float3 GetPosition() const override;

        Quaternion GetRotation() const override;

        bool IsGrounded() const override;

        void Jump(float jumpSpeed) override;

        void SetLinearVelocity(const Float3 velocity) override;

        void SetPosition(const Float3 position) override;

        void SetRotation(const Quaternion rotation) override;

        void Update(float deltaTime) override;

    private:
        JPH::Ref<JPH::CharacterVirtual> m_character;
        JPH::ObjectLayer m_objectLayer;
    };
}
