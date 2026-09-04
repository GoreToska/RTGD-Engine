//
// Created by ivan on 9/3/26.
//

#pragma once

#include <string>
#include <optional>

#include "Tools/Alias.h"

#include <Jolt/Jolt.h>

#include "RigidbodyComponent.h"
#include "Jolt/Core/Reference.h"
#include "Jolt/Physics/Constraints/HingeConstraint.h"
#include "Jolt/Physics/Constraints/TwoBodyConstraint.h"
#include "Scene/SceneManager.h"

namespace RTGDEngine
{
    enum class EConstraintType { Hinge };

    struct ConstraintComponent
    {
        EConstraintType Type = EConstraintType::Hinge;
        std::string OtherEntityName;

        Float3 Point1 = {0, 0, 0}; // world space pivot on THIS body
        Float3 HingeAxis1 = {0, 1, 0};
        Float3 NormalAxis1 = {1, 0, 0};

        Float3 Point2 = {0, 0, 0}; // world space pivot on other body
        Float3 HingeAxis2 = {0, 1, 0};
        Float3 NormalAxis2 = {1, 0, 0};

        bool EnableLimits = false;
        float LimitsMinDeg = -180.0f;
        float LimitsMaxDeg = 180.0f;
        float MaxFrictionTorque = 0.0f;

        // Transient
        JPH::Ref<JPH::TwoBodyConstraint> NativeConstraint;

        static void RegisterMeta(const World& world)
        {
            world.component<EConstraintType>()
                    .constant("Hinge", EConstraintType::Hinge);

            world.component<ConstraintComponent>("ConstraintComponent")
                    .member<EConstraintType>("Type")
                    .member<std::string>("OtherEntityName")
                    .member<Float3>("Point1")
                    .member<Float3>("HingeAxis1")
                    .member<Float3>("NormalAxis1")
                    .member<Float3>("Point2")
                    .member<Float3>("HingeAxis2")
                    .member<Float3>("NormalAxis2")
                    .member<bool>("EnableLimits")
                    .member<float>("LimitsMinDeg")
                    .member<float>("LimitsMaxDeg")
                    .member<float>("MaxFrictionTorque");


            auto createConstraint = [](Entity e)
            {
                auto constraint = e.get_ref<ConstraintComponent>();
                auto rb1 = e.get_ref<RigidbodyComponent>();
                if (!constraint || !rb1 || rb1->BodyID.IsInvalid())
                {
                    LogError("Constraint {}: no self body yet (rb1={}, bodyValid={})",
                             e.name().c_str(), rb1 ? "yes" : "no", rb1 && !rb1->BodyID.IsInvalid());
                    return;
                }

                if (constraint->NativeConstraint)
                {
                    GPhysics().GetJoltSystem().RemoveConstraint(constraint->NativeConstraint);
                    constraint->NativeConstraint = nullptr;
                }

                JPH::BodyID id2;
                if (!constraint->OtherEntityName.empty())
                {
                    Entity other = GScene().Find(constraint->OtherEntityName);
                    if (!other)
                    {
                        LogError("Constraint {}: '{}' not found", e.name().c_str(),
                                 constraint->OtherEntityName.c_str());
                        return;
                    }

                    auto rb2 = other ? other.get_ref<RigidbodyComponent>() : flecs::ref<RigidbodyComponent>{};
                    if (!rb2 || rb2->BodyID.IsInvalid())
                    {
                        LogError("Constraint {}: other body not ready", e.name().c_str());
                        return;
                    }

                    id2 = rb2->BodyID;
                }

                auto& lockInterface = GPhysics().GetJoltSystem().GetBodyLockInterface();
                JPH::BodyLockWrite lock1(lockInterface, rb1->BodyID);
                if (!lock1.Succeeded())
                {
                    LogError("Constraint {}: lock1 failed", e.name().c_str());
                    return;
                }

                std::optional<JPH::BodyLockWrite> lock2;
                JPH::Body* body2 = &JPH::Body::sFixedToWorld;
                if (!id2.IsInvalid())
                {
                    lock2.emplace(lockInterface, id2);
                    if (!lock2->Succeeded())
                    {
                        LogError("Constraint {}: lock2 failed", e.name().c_str());
                        return;
                    }
                    body2 = &lock2->GetBody();
                }

                JPH::HingeConstraintSettings settings;
                settings.mPoint1 = ToRVec3(constraint->Point1);
                settings.mHingeAxis1 = ToVec3(constraint->HingeAxis1);
                settings.mNormalAxis1 = ToVec3(constraint->NormalAxis1);
                settings.mPoint2 = ToRVec3(constraint->Point2);
                settings.mHingeAxis2 = ToVec3(constraint->HingeAxis2);
                settings.mNormalAxis2 = ToVec3(constraint->NormalAxis2);
                settings.mMaxFrictionTorque = constraint->MaxFrictionTorque;
                if (constraint->EnableLimits)
                {
                    settings.mLimitsMin = constraint->LimitsMinDeg * (JPH::JPH_PI / 180.0f);
                    settings.mLimitsMax = constraint->LimitsMaxDeg * (JPH::JPH_PI / 180.0f);
                }

                constraint->NativeConstraint = settings.Create(lock1.GetBody(), *body2);
                GPhysics().GetJoltSystem().AddConstraint(constraint->NativeConstraint);
                LogInfo("Constraint {}: built, ptr={}", e.name().c_str(), (void*)constraint->NativeConstraint.GetPtr());
            };

            world.observer<ConstraintComponent>().event(flecs::OnRemove).each(
                [createConstraint](Entity e, ConstraintComponent& c)
                {
                    if (c.NativeConstraint)
                        GPhysics().GetJoltSystem().RemoveConstraint(c.NativeConstraint);
                });
        }
    };
}
