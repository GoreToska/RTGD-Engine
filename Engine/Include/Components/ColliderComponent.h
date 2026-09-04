//
// Created by ivan on 8/25/26.
//

#pragma once
#include <Jolt/Jolt.h>

#include "TransformComponent.h"
#include "AssetLoader/MeshImporter.h"
#include "AssetLoader/MeshSimplifier.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Collision/Shape/ConvexHullShape.h"
#include "Jolt/Physics/Collision/Shape/MeshShape.h"
#include "Jolt/Physics/Collision/Shape/ScaledShape.h"
#include "Jolt/Physics/Collision/Shape/Shape.h"
#include "Jolt/Physics/Collision/Shape/SphereShape.h"
#include "Tools/Alias.h"
#include "Tools/JoltConversions.h"
#include "Tools/Logger.h"
#include "AssetLoader/PathResolve.h"

namespace RTGDEngine
{
    enum class EPhysicsShape
    {
        Box,
        Sphere,
        Capsule,
        Mesh,
        ConvexHull,
    };

    struct ColliderComponent
    {
        EPhysicsShape Shape = EPhysicsShape::Box;
        Float3 Extents = {0.5f, 0.5f, 0.5f};
        std::string CollisionMeshPath; // Relative path to mesh
        uint32_t ConvexHullPointTarget = 50; // Used for mesh simplification for convex colliders
        uint32_t MeshTriangleTarget = 500; // Used for mesh simplification for mesh colliders
        float Friction = 0.2f;
        float Restitution = 0.0f;
        bool IsTrigger = false;
        uint8_t Layer = 0;

        // Transient
        JPH::ShapeRefC NativeShape;

        static JPH::ShapeRefC MakeShape(EPhysicsShape shape, const Float3& extents)
        {
            switch (shape)
            {
                case EPhysicsShape::Box:
                    return JPH::BoxShapeSettings(ToVec3(extents)).Create().Get();
                case EPhysicsShape::Sphere:
                    return JPH::SphereShapeSettings(extents.x).Create().Get();
                case EPhysicsShape::Capsule:
                    return JPH::CapsuleShapeSettings(extents.x, extents.y).Create().Get();
            }

            return nullptr;
        }

        static JPH::ShapeRefC MakeMeshShape(EPhysicsShape shape, const std::string& absolutePath, const Float3& scale,
                                            const uint32_t ConvexHullPointTarget = 200,
                                            const uint32_t MeshTriangleTarget = 1000)
        {
            MeshImportData data = GMeshImporter().Import(absolutePath);
            if (!data.Success)
            {
                LogError("Collision mesh import failed '{}': {}", absolutePath, data.ErrorMessage);
                return nullptr;
            }

            JPH::Shape::ShapeResult result;

            if (shape == EPhysicsShape::Mesh)
            {
                if (MeshTriangleTarget > 0 && data.Indices.size() / 3 > MeshTriangleTarget)
                {
                    float ratio = static_cast<float>(MeshTriangleTarget) * 3.0f / static_cast<float>(data.Indices.
                                      size());
                    data = GMeshSimplifier().Simplify(data, ratio, 0.1);
                }

                JPH::VertexList vertices;
                vertices.reserve(data.Vertices.size());
                for (auto& v: data.Vertices)
                    vertices.push_back(JPH::Float3(v.Position.x, v.Position.y, v.Position.z));

                JPH::IndexedTriangleList triangles;
                triangles.reserve(data.Indices.size() / 3);
                for (size_t i = 0; i + 2 < data.Indices.size(); i += 3)
                    triangles.push_back(
                        JPH::IndexedTriangle(data.Indices[i], data.Indices[i + 1], data.Indices[i + 2], 0));

                result = JPH::MeshShapeSettings(std::move(vertices), std::move(triangles)).Create();
            }
            else
            {
                std::vector<VertexPNTUV>* src = &data.Vertices;
                std::vector<Float3> simplified;
                if (data.Vertices.size() > ConvexHullPointTarget)
                {
                    simplified = GMeshSimplifier().SimplifyPoints(data.Vertices, ConvexHullPointTarget);
                }

                JPH::Array<JPH::Vec3> points;
                if (!simplified.empty())
                {
                    points.reserve(simplified.size());
                    for (auto& p: simplified)
                        points.push_back(ToVec3(p));
                }
                else
                {
                    points.reserve(data.Vertices.size());
                    for (auto& v: data.Vertices)
                        points.push_back(ToVec3(v.Position));
                }

                result = JPH::ConvexHullShapeSettings(points).Create();
            }

            if (!result.IsValid())
            {
                LogError("Collision shape build failed '{}': {}", absolutePath, result.GetError());
                return nullptr;
            }

            JPH::ShapeRefC finalShape = result.Get();
            if (scale.x != 1.0f || scale.y != 1.0f || scale.z != 1.0f)
            {
                auto scaled = JPH::ScaledShapeSettings(finalShape, ToVec3(scale)).Create();
                if (!scaled.IsValid())
                {
                    LogError("Collision shape scale failed '{}': {}", absolutePath, scaled.GetError());
                    return nullptr;
                }

                finalShape = scaled.Get();
            }

            return finalShape;
        }

        static JPH::MassProperties ComputeMassProperties(EPhysicsShape shape, const Float3& extents, float mass)
        {
            JPH::MassProperties props = MakeShape(shape, extents)->GetMassProperties();
            props.ScaleToMass(mass);
            return props;
        }

        static void RegisterMeta(const World& world)
        {
            world.component<EPhysicsShape>("PhysicsShape")
                    .constant("Box", EPhysicsShape::Box)
                    .constant("Sphere", EPhysicsShape::Sphere)
                    .constant("Capsule", EPhysicsShape::Capsule)
                    .constant("Mesh", EPhysicsShape::Mesh)
                    .constant("ConvexHull", EPhysicsShape::ConvexHull);

            world.component<ColliderComponent>("ColliderComponent")
                    .member<EPhysicsShape>("Shape")
                    .member<Float3>("Extents")
                    .member<std::string>("CollisionMeshPath")
                    .member<uint32_t>("ConvexHullPointTarget")
                    .member<uint32_t>("MeshTriangleTarget")
                    .member<float>("Friction")
                    .member<float>("Restitution")
                    .member<bool>("IsTrigger")
                    .member<uint8_t>("Layer");

            auto buildShape = [](flecs::entity e)
            {
                auto collider = e.get_ref<ColliderComponent>();
                if (!collider)
                    return;

                bool isMeshShape = collider->Shape == EPhysicsShape::Mesh || collider->Shape ==
                                   EPhysicsShape::ConvexHull;

                if (isMeshShape)
                {
                    auto transform = e.get_ref<TransformComponent>();
                    if (!transform)
                        return;

                    collider->NativeShape = MakeMeshShape(collider->Shape, GetAbsolutePath(collider->CollisionMeshPath),
                                                          transform->Scale, collider->ConvexHullPointTarget,
                                                          collider->MeshTriangleTarget);
                }
                else
                {
                    collider->NativeShape = MakeShape(collider->Shape, collider->Extents);
                }
            };

            world.observer<ColliderComponent>().event(flecs::OnSet).each(
                [buildShape](flecs::entity e, ColliderComponent&)
                {
                    buildShape(e);
                });

            world.observer<TransformComponent>().event(flecs::OnSet).each(
                [buildShape](flecs::entity e, TransformComponent&)
                {
                    auto collider = e.get_ref<ColliderComponent>();
                    if (collider && !collider->NativeShape)
                        buildShape(e);
                });
        }
    };
}
