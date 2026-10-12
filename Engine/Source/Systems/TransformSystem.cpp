//
// Created by ivan on 9/30/26.
//

#include "Systems/TransformSystem.h"

#include "Components/TransformComponent.h"

namespace RTGDEngine
{
    void TransformSystem::Update(World& world)
    {
        if (!m_propagate.is_alive())
            m_propagate = world.system<TransformComponent, const TransformComponent*>("TransformPropagation")
                    .kind(0).term_at(1).parent().cascade().each(
                        [](TransformComponent& t, const TransformComponent* parent)
                        {
                            const Matrix4 local = t.GetLocalMatrix();
                            const Quaternion localRotation = Diligent::normalize(t.Rotation);

                            t.WorldMatrix = parent ? local * parent->WorldMatrix : local;
                            t.WorldRotation = parent ? parent->WorldRotation * localRotation : localRotation;

                            const Matrix4& m = t.WorldMatrix;
                            t.WorldPosition = {m._41, m._42, m._43};
                            t.WorldScale = {
                                Diligent::length(Float3{m._11, m._12, m._13}),
                                Diligent::length(Float3{m._21, m._22, m._23}),
                                Diligent::length(Float3{m._31, m._32, m._33})
                            };
                        });

        m_propagate.run();
    }
} // RTGDEngine
