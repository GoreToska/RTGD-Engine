//
// Created by gorev on 12.03.2026.
//

#pragma once
#include "AssetLoader/AssetManager.h"
#include "AssetLoader/Refs/AssetRef.h"
#include "Render/RenderHandle.h"
#include "AssetLoader/PathResolve.h"

namespace RTGDEngine {
    // Important!
    // Never write MeshRef & MaterialRef handles like Mesh.Handle = MyHandle
    // ECS-like setter should be used:
    //      entity.set(MeshComponent{ MeshRef{"MyModel.gltf"}, MyMaterial);
    // or in place:
    //      e.get_ref<MeshComponent>()->Mesh.Path = "MyModel.gltf";
    //      e.modified<MeshComponent>();   // this will emit OnSet

    struct MeshComponent {
        MeshRef Mesh;
        MaterialRef Material;

        MeshComponent() = default;

        MeshComponent(const MeshRef &meshRef, const MaterialRef &material)
            : Mesh(meshRef), Material(material) {
        }

        static void RegisterMeta(const flecs::world &world) {
            flecs::component<MeshComponent>(world, "MeshComponent")
                    .member<MeshRef>("Mesh")
                    .member<MaterialRef>("Material");

            world.observer<MeshComponent>()
                    .event(flecs::OnSet)
                    .each([](MeshComponent &mc) {
                        auto &am = AssetManager::Instance();
                        if (!mc.Mesh.Path.empty())
                            mc.Mesh.Resolve(am.GetMesh(GetAbsolutePath(mc.Mesh.Path)));
                        if (!mc.Material.Path.empty())
                            mc.Material.Resolve(am.GetMaterial(GetAbsolutePath(mc.Material.Path)));
                    });
        }
    };
}
