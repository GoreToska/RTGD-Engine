//
// Created by gorev on 27.03.2026.
//
#include "Engine/Reflection.h"

#include "Components/CameraComponent.h"
#include "Components/LightComponent.h"
#include "Components/MeshComponent.h"
#include "Components/RenderComponent.h"
#include "Components/TransformComponent.h"
#include "Tools/Alias.h"
#include "Tools/Logger.h"

namespace RTGDEngine
{
    void RegisterReflectionTypes(flecs::world& world)
    {
        world.component<Float2>()
                .member<float>("X", offsetof(Float2, x))
                .member<float>("Y", offsetof(Float2, y));

        world.component<Float3>()
                .member<float>("X", offsetof(Float3, x))
                .member<float>("Y", offsetof(Float3, y))
                .member<float>("Z", offsetof(Float3, z));

        world.component<Float4>()
                .member<float>("X", offsetof(Float4, x))
                .member<float>("Y", offsetof(Float4, y))
                .member<float>("Z", offsetof(Float4, z))
                .member<float>("W", offsetof(Float4, w));

        /*world.component<Quaternion>()
                .member<float>("X", offsetof(Quaternion, q[0]))
                .member<float>("Y", offsetof(Quaternion, q[1]))
                .member<float>("Z", offsetof(Quaternion, q[2]))
                .member<float>("W", offsetof(Quaternion, q[3]));*/

        world.component<TransformComponent>()
                .member<Float3>("Position", offsetof(TransformComponent, Position))
                .member<Quaternion>("Rotation", offsetof(TransformComponent, Rotation))
                .member<Float3>("Scale", offsetof(TransformComponent, Scale));

        world.component<MeshComponent>()
                .member<uint32_t>("MeshHandle", offsetof(MeshComponent, meshHandle))
                .member<uint32_t>("MaterialHandle", offsetof(MeshComponent, materialHandle));

        world.component<RenderComponent>()
                .member<bool>("IsVisible", offsetof(RenderComponent, IsVisible));

        world.component<CameraComponent>()
                .member<float>("FOVDegrees", offsetof(CameraComponent, FOVDegrees))
                .member<float>("AspectRatio", offsetof(CameraComponent, AspectRatio))
                .member<float>("NearPlane", offsetof(CameraComponent, NearPlane))
                .member<float>("FarPlane", offsetof(CameraComponent, FarPlane));

        world.component<AmbientLightComponent>()
                .member<Float3>("Color", offsetof(AmbientLightComponent, Color))
                .member<float>("Intensity", offsetof(AmbientLightComponent, Intensity));

        world.component<DirectionalLightComponent>()
                .member<Float3>("Direction", offsetof(DirectionalLightComponent, Direction))
                .member<Float3>("Color", offsetof(DirectionalLightComponent, Color))
                .member<float>("Intensity", offsetof(DirectionalLightComponent, Intensity));

        world.component<PointLightComponent>()
                .member<Float3>("Color", offsetof(PointLightComponent, Color))
                .member<float>("Intensity", offsetof(PointLightComponent, Intensity))
                .member<float>("Radius", offsetof(PointLightComponent, Radius));

        world.component<SpotLightComponent>()
                .member<Float3>("Direction", offsetof(SpotLightComponent, Direction))
                .member<Float3>("Color", offsetof(SpotLightComponent, Color))
                .member<float>("Intensity", offsetof(SpotLightComponent, Intensity))
                .member<float>("InnerAngle", offsetof(SpotLightComponent, InnerAngle))
                .member<float>("OuterAngle", offsetof(SpotLightComponent, OuterAngle))
                .member<float>("Radius", offsetof(SpotLightComponent, Radius));

        LogInfo("Reflection initialized");
    }
}
