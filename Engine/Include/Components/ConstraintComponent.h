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
    enum class EConstraintType
    {
        Hinge,
        Slider,
        Fixed,
        Distance,
        Cone,
        SwingTwist
    };

    struct ConstraintComponent
    {
        EConstraintType Type = EConstraintType::Hinge;
        std::string OtherEntityName;

        Float3 Point1 = {0, 0, 0}; // world space pivot on THIS body
        Float3 Axis1 = {0, 1, 0};
        Float3 NormalAxis1 = {1, 0, 0};

        Float3 Point2 = {0, 0, 0}; // world space pivot on other body
        Float3 Axis2 = {0, 1, 0};
        Float3 NormalAxis2 = {1, 0, 0};

        bool EnableLimits = false;
        float LimitsMin = -180.0f;
        float LimitsMax = 180.0f;
        float HalfConeAngle = 0.0f;
        float PlaneHalfConeAngle = 0.0f;

        float MinDistance = -1.0f;
        float MaxDistance = -1.0f;
        float MaxFriction = 0.0f;

        // Transient
        JPH::Ref<JPH::TwoBodyConstraint> NativeConstraint;

        static void RegisterMeta(const World& world)
        {
            world.component<EConstraintType>()
                    .constant("Hinge", EConstraintType::Hinge)
                    .constant("Slider", EConstraintType::Slider)
                    .constant("Fixed", EConstraintType::Fixed)
                    .constant("Distance", EConstraintType::Distance)
                    .constant("Cone", EConstraintType::Cone)
                    .constant("SwingTwist", EConstraintType::SwingTwist);

            world.component<ConstraintComponent>("ConstraintComponent")
                    .member<EConstraintType>("Type")
                    .member<std::string>("OtherEntityName")
                    .member<Float3>("Point1")
                    .member<Float3>("Axis1")
                    .member<Float3>("NormalAxis1")
                    .member<Float3>("Point2")
                    .member<Float3>("Axis2")
                    .member<Float3>("NormalAxis2")
                    .member<bool>("EnableLimits")
                    .member<float>("LimitsMin")
                    .member<float>("LimitsMax")
                    .member<float>("HalfConeAngle")
                    .member<float>("PlaneHalfConeAngle")
                    .member<float>("MinDistance")
                    .member<float>("MaxDistance")
                    .member<float>("MaxFriction");

            world.observer<ConstraintComponent>().event(flecs::OnRemove).each(
                [](Entity e, ConstraintComponent& c)
                {
                    if (c.NativeConstraint)
                        GPhysics().GetJoltSystem().RemoveConstraint(c.NativeConstraint);
                });
        }
    };
}
