//
// Created by gorev on 17.03.2026.
//

#include "Systems/LightSystem.h"

#include "Components/LightComponent.h"
#include "Components/TransformComponent.h"
#include "Render/ConstBuffers.h"
#include "Render/RenderSystem.h"

namespace RTGDEngine
{
    void LightSystem::Update(const flecs::world& world)
    {
        UpdateAmbient(world);
        UpdateDirectionalLights(world);
        UpdatePointLights(world);
        UpdateSpotLights(world);

        RTGDRenderSystem::Instance().GetFrameConstants().UpdateLight(m_lightCB);
    }

    void LightSystem::UpdateAmbient(const flecs::world& world)
    {
        world.each([&](const AmbientLightComponent& ambient)
        {
            m_lightCB.AmbientColor = ambient.Color;
            m_lightCB.AmbientIntensity = ambient.Intensity;
        });
    }

    void LightSystem::UpdateDirectionalLights(const flecs::world& world)
    {
        world.each([&](const DirectionalLightComponent& light)
        {
            if (m_lightCB.DirectionalCount >= MAX_DIRECTIONAL_LIGHTS)
                return;

            auto& dst = m_lightCB.DirectionalLights[m_lightCB.DirectionalCount++];
            dst.Direction = Diligent::normalize(light.Direction);
            dst.Color = light.Color;
            dst.Intensity = light.Intensity;
        });
    }

    void LightSystem::UpdatePointLights(const flecs::world& world)
    {
        world.each([&](const PointLightComponent& light,
                       const TransformComponent& transform)
        {
            if (m_lightCB.PointCount >= MAX_POINT_LIGHTS)
                return;

            auto& dst = m_lightCB.PointLights[m_lightCB.PointCount++];
            dst.Position = transform.Position;
            dst.Color = light.Color;
            dst.Intensity = light.Intensity;
            dst.Radius = light.Radius;
        });
    }

    void LightSystem::UpdateSpotLights(const flecs::world& world)
    {
        world.each([&](const SpotLightComponent& light,
                       const TransformComponent& transform)
        {
            if (m_lightCB.SpotCount >= MAX_SPOT_LIGHTS)
                return;

            auto& dst = m_lightCB.SpotLights[m_lightCB.SpotCount++];
            dst.Position = transform.Position;
            dst.Direction = Diligent::normalize(light.Direction);
            dst.Color = light.Color;
            dst.Intensity = light.Intensity;
            dst.Radius = light.Radius;
            dst.InnerAngle = light.InnerAngle * Diligent::PI_F / 180.0f;
            dst.OuterAngle = light.OuterAngle * Diligent::PI_F / 180.0f;
        });
    }
} // RTGDEngine
