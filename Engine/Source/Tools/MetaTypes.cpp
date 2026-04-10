//
// Created by gorev on 05.04.2026.
//
// ReSharper disable CppExpressionWithoutSideEffects
#include "Tools/MetaTypes.h"

#include "Components/CameraComponent.h"
#include "Components/GameObject.h"
#include "Components/LightComponent.h"
#include "Components/MeshComponent.h"
#include "Components/RenderComponent.h"
#include "Components/TransformComponent.h"
#include "Components/UUIDComponent.h"
#include "Components/VelocityComponent.h"
#include "Tools/Alias.h"
#include "Tools/Logger.h"

namespace RTGDEngine
{
    void RegisterMetaTypes(const flecs::world& world)
    {
        world.component<Float2>()
                .member<float>("x", offsetof(Float2, x))
                .member<float>("y", offsetof(Float2, y));

        world.component<Float3>()
                .member<float>("x", offsetof(Float3, x))
                .member<float>("y", offsetof(Float3, y))
                .member<float>("z", offsetof(Float3, z));

        world.component<Float4>()
                .member<float>("x", offsetof(Float4, x))
                .member<float>("y", offsetof(Float4, y))
                .member<float>("z", offsetof(Float4, z))
                .member<float>("w", offsetof(Float4, w));

        world.component<Quaternion>()
                .member<float>("x", offsetof(Quaternion, q.x))
                .member<float>("y", offsetof(Quaternion, q.y))
                .member<float>("z", offsetof(Quaternion, q.z))
                .member<float>("w", offsetof(Quaternion, q.w))
                .set_doc_brief("Quaternion (x, y, z, w)")
                .set_doc_detail("Rotation represented as a unit quaternion");

        TransformComponent::RegisterMeta(world);
        CameraComponent::RegisterMeta(world);
        EditorCameraMovementComponent::RegisterMeta(world);
        GameObject::RegisterMeta(world);
        AmbientLightComponent::RegisterMeta(world);
        DirectionalLightComponent::RegisterMeta(world);
        SpotLightComponent::RegisterMeta(world);
        PointLightComponent::RegisterMeta(world);
        MeshComponent::RegisterMeta(world);
        RenderComponent::RegisterMeta(world);
        UUIDComponent::RegisterMeta(world);
        VelocityComponent::RegisterMeta(world);
    }
}
