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

namespace RTGDEngine {
    void RegisterMetaTypes(const flecs::world &world) {
        world.component<std::string>()
                .opaque(flecs::String)
                .serialize([](const flecs::serializer *s, const std::string *data) {
                    const char *str = data->c_str();
                    return s->value(flecs::String, &str);
                })
                .assign_string([](std::string *data, const char *value) {
                    *data = value;
                });

        flecs::component<MeshRef>(world, "AssetRef")
                .member<std::string>("Path"); // handle is transient
        flecs::component<MaterialRef>(world, "MaterialRef")
                .member<std::string>("Path"); // handle is transient

        if (!MetaAlreadyRegistered(world, flecs::component<Float2>(world, "Float2")))
            world.component<Float2>()
                    .member<float>("x")
                    .member<float>("y");

        if (!MetaAlreadyRegistered(world, flecs::component<Float3>(world, "Float3")))
            world.component<Float3>()
                    .member<float>("x")
                    .member<float>("y")
                    .member<float>("z");

        if (!MetaAlreadyRegistered(world, flecs::component<Float4>(world, "Float4")))
            world.component<Float4>()
                    .member<float>("x")
                    .member<float>("y")
                    .member<float>("z")
                    .member<float>("w");

        if (!MetaAlreadyRegistered(world, flecs::component<Quaternion>(world, "Quaternion")))
            world.component<Quaternion>()
                    .member<float>("x")
                    .member<float>("y")
                    .member<float>("z")
                    .member<float>("w")
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

        LogInfo("Meta types registered");
    }
}
