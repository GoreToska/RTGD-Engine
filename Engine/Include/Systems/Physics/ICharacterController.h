//
// Created by ivan on 8/28/26.
//

#pragma once
#include "Tools/Alias.h"

namespace RTGDEngine
{
    enum class ECharacterGroundState
    {
        OnGround,
        OnSteepGround,
        NotSupported,
        InAir,
    };

    class ICharacterController
    {
    public:
        virtual ~ICharacterController() = default;

        virtual void SetLinearVelocity(const Float3 velocity) = 0;

        virtual void Jump(float jumpSpeed) = 0;

        virtual Float3 GetLinearVelocity() const = 0;

        virtual Float3 GetPosition() const = 0;

        virtual void SetPosition(const Float3 position) = 0;

        virtual Quaternion GetRotation() const = 0;

        virtual void SetRotation(const Quaternion rotation) = 0;

        virtual ECharacterGroundState GetGroundState() const = 0;

        virtual bool IsGrounded() const = 0;

        virtual Float3 GetGroundNormal() const = 0;

        virtual Float3 GetGroundVelocity() const = 0;

        virtual void Update(float deltaTime) = 0;
    };
} // RTGDEngine
